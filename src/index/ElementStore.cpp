#include "BoundingBox.hpp"
#include "GeoCoordinate.hpp"
#include "QuadKey.hpp"
#include "entities/Element.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "entities/ElementVisitor.hpp"
#include "formats/FormatTypes.hpp"
#include "index/ElementStore.hpp"
#include "mapcss/Style.hpp"
#include "mapcss/StyleProvider.hpp"
#include "utils/GeoUtils.hpp"
#include "clipper/clipper.hpp"

using namespace utymap;
using namespace utymap::entities;
using namespace utymap::formats;
using namespace utymap::mapcss;

namespace utymap { namespace index {

const static double Scale = 1E8; // max precision for Lat/Lon
const static std::string ClipKey = "clip";
const static std::string SkipKey = "clip";

// Creates bounding box of given element.
struct BoundingBoxVisitor : public ElementVisitor
{
    BoundingBox boundingBox;

    void visitNode(const Node& node)
    {
        boundingBox.expand(node.coordinate);
    }

    void visitWay(const Way& way)
    {
        boundingBox.expand(way.coordinates.cbegin(), way.coordinates.cend());
    }

    void visitArea(const Area& area)
    {
        boundingBox.expand(area.coordinates.cbegin(), area.coordinates.cend());
    }

    void visitRelation(const Relation& relation)
    {
        for (const auto& element: relation.elements) {
            element->accept(*this);
        }
    }
};

// Modifies geometry of element by bounding box clipping
class ElementGeometryVisitor : private ElementVisitor
{
public:

    // Defines polygon points location relative to current quadkey.
    enum PointLocation { AllInside, AllOutside, Mixed };

    ElementGeometryVisitor(ElementStore& elementStore) :
        elementStore_(elementStore),
        quadKeyPtr_(nullptr),
        quadKeyBboxPtr_(nullptr)
    {
    }

    void clipAndStore(const Element& element, const QuadKey& quadKey, const BoundingBox& quadKeyBbox)
    {
        quadKeyPtr_ = &quadKey;
        quadKeyBboxPtr_ = &quadKeyBbox;
        element.accept(*this);
    }

    void visitNode(const Node& node)
    {
        // here, node should be always in tile
        elementStore_.storeImpl(node, *quadKeyPtr_);
    }

    void visitWay(const Way& way)
    {
        ClipperLib::Path wayShape;
        PointLocation pointLocation = setPath(*quadKeyBboxPtr_, way, wayShape);
        // 1. all geometry inside current quadkey: no need to truncate.
        if (pointLocation == PointLocation::AllInside) {
            elementStore_.storeImpl(way, *quadKeyPtr_);
            return;
        }

        // 2. all geometry outside : way should be skipped
        if (pointLocation == PointLocation::AllOutside) {
            return;
        }

        ClipperLib::PolyTree solution;
        clipper_.AddPath(wayShape, ClipperLib::ptSubject, false);
        clipper_.AddPath(createPathFromBoundingBox(), ClipperLib::ptClip, true);
        clipper_.Execute(ClipperLib::ctIntersection, solution);
        clipper_.Clear();
        int count = solution.Total();

        // 3. way intersects border only once: store a copy with clipped geometry
        if (count == 1) {
            Way clippedWay;
            setData(clippedWay, way, solution.GetFirst()->Contour);
            elementStore_.storeImpl(clippedWay, *quadKeyPtr_);
        }
        // 4. in this case, result should be stored as relation (collection of ways)
        else {
            Relation relation;
            relation.id = way.id;
            relation.tags = way.tags;
            relation.elements.reserve(count);
            ClipperLib::PolyNode* polyNode = solution.GetFirst();
            while (polyNode) {
                std::shared_ptr<Way> clippedWay(new Way());
                clippedWay->id = way.id;
                setCoordinates(*clippedWay, polyNode->Contour);
                relation.elements.push_back(clippedWay);
                polyNode = polyNode->GetNext();
            }
            elementStore_.storeImpl(relation, *quadKeyPtr_);
        }
    }

    void visitArea(const Area& area)
    {
        ClipperLib::Path areaShape;
        PointLocation pointLocation = setPath(*quadKeyBboxPtr_, area, areaShape);
        // 1. all geometry inside current quadkey: no need to truncate.
        if (pointLocation == PointLocation::AllInside) {
            elementStore_.storeImpl(area, *quadKeyPtr_);
            return;
        }

        // 2. all geometry outside : pass empty
        if (pointLocation == PointLocation::AllOutside) {
            Area emptyArea;
            emptyArea.id = area.id;
            emptyArea.tags = area.tags;
            elementStore_.storeImpl(emptyArea, *quadKeyPtr_);
            return;
        }

        ClipperLib::Paths solution;
        clipper_.AddPath(areaShape, ClipperLib::ptSubject, true);
        clipper_.AddPath(createPathFromBoundingBox(), ClipperLib::ptClip, true);
        clipper_.Execute(ClipperLib::ctIntersection, solution);
        clipper_.Clear();

        // 3. way intersects border only once: store a copy with clipped geometry
        if (solution.size() == 1) {
            Area clippedArea;
            setData(clippedArea, area, solution[0]);
            elementStore_.storeImpl(clippedArea, *quadKeyPtr_);
        }
        // 4. in this case, result should be stored as relation (collection of areas)
        else {
            Relation relation;
            relation.id = area.id;
            relation.tags = area.tags;
            relation.elements.reserve(solution.size());
            for (auto it = solution.begin(); it != solution.end(); ++it) {
                std::shared_ptr<Area> clippedArea(new Area());
                clippedArea->id = area.id;
                setCoordinates(*clippedArea, *it);
                relation.elements.push_back(clippedArea);
            }
            elementStore_.storeImpl(relation, *quadKeyPtr_);
        }
    }

    void visitRelation(const Relation& relation)
    {
        struct RelationVisitor : public ElementVisitor
        {
            Relation data;

            RelationVisitor(const BoundingBox& quadKeyBbox, ClipperLib::Clipper& clipper) :
                bbox_(quadKeyBbox), clipper_(clipper) { }

            void visitNode(const Node& node)
            {
                if (bbox_.contains(node.coordinate)) {
                    data.elements.push_back(std::shared_ptr<Node>(new Node(node)));
                }
            }

            void visitWay(const Way& way)
            {
                ClipperLib::Path wayShape;
                ElementGeometryVisitor::setPath(bbox_, way, wayShape);
                clipper_.AddPath(wayShape, ClipperLib::ptSubject, false);
            }

            void visitArea(const Area& area)
            {
                ClipperLib::Path areaShape;
                ElementGeometryVisitor::setPath(bbox_, area, areaShape);
                clipper_.AddPath(areaShape, ClipperLib::ptSubject, true);
            }

            void visitRelation(const Relation& relation)
            {
                for (const auto& element : relation.elements) {
                    element->accept(*this);
                }
            }

        private:
            const BoundingBox& bbox_;
            ClipperLib::Clipper& clipper_;

        } visitor(*quadKeyBboxPtr_, clipper_);

        relation.accept(visitor);

        // Process results
        ClipperLib::PolyTree solution;
        clipper_.AddPath(createPathFromBoundingBox(), ClipperLib::ptClip, true);
        clipper_.Execute(ClipperLib::ctIntersection, solution);
        clipper_.Clear();

        int count = solution.Total();
        // Do not store one result as relation
        if (count == 1) {
            ClipperLib::PolyNode* node = solution.GetFirst();
            if (node->IsOpen()) {
                Way way;
                setData(way, relation, node->Contour);
                elementStore_.storeImpl(way, *quadKeyPtr_);
            }
            else {
                if (!node->IsHole()) {
                    Area area;
                    setData(area, relation, node->Contour);
                    elementStore_.storeImpl(area, *quadKeyPtr_);
                }
            }
        }
        else if (count > 1) {
            Relation newRelation;
            newRelation.id = relation.id;
            newRelation.tags = relation.tags;
            newRelation.elements.reserve(count);

            ClipperLib::PolyNode* polyNode = solution.GetFirst();
            while (polyNode) {
                if (polyNode->IsOpen()) {
                    std::shared_ptr<Way> way(new Way());
                    setCoordinates(*way, polyNode->Contour);
                    newRelation.elements.push_back(way);
                }
                else {
                    std::shared_ptr<Area> area(new Area());
                    setCoordinates(*area, polyNode->Contour);
                    newRelation.elements.push_back(area);
                }
                polyNode = polyNode->GetNext();
            }
            elementStore_.storeImpl(newRelation, *quadKeyPtr_);
        }
    }

    template<typename T>
    inline static PointLocation setPath(const BoundingBox& bbox, const T& t, ClipperLib::Path& shape) {
        shape.reserve(t.coordinates.size());
        bool allInside = false;
        bool allOutside = true;
        for (const GeoCoordinate& coord : t.coordinates) {
            bool contains = bbox.contains(coord);
            allInside &= contains;
            allOutside &= !contains;
            shape.push_back(ClipperLib::IntPoint(coord.longitude*Scale, coord.latitude*Scale));
        }

        return allInside ? PointLocation::AllInside :
              (allOutside ? PointLocation::AllOutside : PointLocation::Mixed);
    }

private:

    template<typename T>
    inline void setData(T& t, const Element& element, const ClipperLib::Path& path) {
        t.id = element.id;
        t.tags = element.tags;
        setCoordinates<T>(t, path);
    }

    template<typename T>
    inline void setCoordinates(T& t, const ClipperLib::Path& path) {
        t.coordinates.reserve(path.size());
        for (const auto& c : path) {
            t.coordinates.push_back(GeoCoordinate(c.Y / Scale, c.X / Scale));
        }
    }

    inline ClipperLib::Path createPathFromBoundingBox()
    {
        double xMin = quadKeyBboxPtr_->minPoint.longitude, yMin = quadKeyBboxPtr_->minPoint.latitude,
            xMax = quadKeyBboxPtr_->maxPoint.longitude, yMax = quadKeyBboxPtr_->maxPoint.latitude;
        ClipperLib::Path rect;
        rect.push_back(ClipperLib::IntPoint(xMin*Scale, yMin*Scale));
        rect.push_back(ClipperLib::IntPoint(xMax*Scale, yMin*Scale));
        rect.push_back(ClipperLib::IntPoint(xMax*Scale, yMax*Scale));
        rect.push_back(ClipperLib::IntPoint(xMin*Scale, yMax*Scale));
        return std::move(rect);
    }

    ElementStore& elementStore_;
    const QuadKey* quadKeyPtr_;
    const BoundingBox* quadKeyBboxPtr_;
    ClipperLib::Clipper clipper_;
};

ElementStore::ElementStore(StringTable& stringTable) : 
    clipKeyId_(stringTable.getId(ClipKey)), skipKeyId_(stringTable.getId(SkipKey))
{
}

ElementStore::~ElementStore()
{
}

bool ElementStore::store(const Element& element, const utymap::index::LodRange& range, const StyleProvider& styleProvider)
{
    BoundingBoxVisitor bboxVisitor;
    bool wasStored = false;
    for (int lod = range.start; lod <= range.end; ++lod) {
        if (!styleProvider.hasStyle(element, lod)) 
            continue;
        Style style = styleProvider.forElement(element, lod);
        if (style.has(skipKeyId_))
            continue;

        // initialize bounding box only once
        if (!bboxVisitor.boundingBox.isValid()) {
            element.accept(bboxVisitor);
        }       
        storeInTileRange(element, bboxVisitor.boundingBox, lod, style.has(clipKeyId_));
        wasStored = true;
    }

    // NOTE still might be clipped and then skipped
    return wasStored;
}

void ElementStore::storeInTileRange(const Element& element, const BoundingBox& elementBbox, int levelOfDetails, bool shouldClip)
{
    ElementGeometryVisitor geometryClipper(*this);
    auto visitor = [&](const QuadKey& quadKey, const BoundingBox& quadKeyBbox) {
        if (shouldClip)
            geometryClipper.clipAndStore(element, quadKey, quadKeyBbox);
        else
            storeImpl(element, quadKey);
    };
    utymap::utils::GeoUtils::visitTileRange(elementBbox, levelOfDetails, visitor);
}

}}
