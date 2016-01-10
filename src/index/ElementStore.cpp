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
#include "index/GeoUtils.hpp"
#include "mapcss/Style.hpp"
#include "mapcss/StyleProvider.hpp"
#include "meshing/clipper.hpp"

using namespace utymap;
using namespace utymap::entities;
using namespace utymap::formats;
using namespace utymap::mapcss;

namespace utymap { namespace index {

class ElementGeometryClipper
{
public:

    const BoundingBox* quadKeyBboxPtr;

    ElementGeometryClipper():
        quadKeyBboxPtr(nullptr)
    {
    }

    void clipNode(const Node& node, const std::function<void(std::shared_ptr<Element>)>& callback)
    {
        if (quadKeyBboxPtr->contains(node.coordinate)) {
            callback(std::shared_ptr<Node>(new Node(node)));
        }
    }

    void clipWay(const Way& way, const std::function<void(std::shared_ptr<Element>)>& callback)
    {
        ClipperLib::Path wayShape;
        PointLocation pointLocation = setPath(way, wayShape);
        // 1. all geometry inside current quadkey: no need to truncate.
        if (pointLocation == PointLocation::AllInside) {
            callback(std::shared_ptr<Way>(new Way(way)));
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

        // 3. way intersects border only once: store a copy with clipped geometry
        if (solution.ChildCount() == 1) {
            ClipperLib::PolyNode* node = solution.Childs[0];
            std::shared_ptr<Way> clippedWay = std::shared_ptr<Way>(new Way());
            clippedWay->id = way.id;
            clippedWay->tags = way.tags;
            setCoordinates(*clippedWay, node->Contour);
            callback(clippedWay);
        }
        // 4. in this case, result should be stored as relation (collection of ways)
        else {
            auto relation = std::shared_ptr<Relation>(new Relation());
            relation->id = way.id;
            relation->tags = way.tags;
            relation->elements.reserve(solution.ChildCount());
            for (auto it = solution.Childs.begin(); it != solution.Childs.end(); ++it) {
                std::shared_ptr<Way> clippedWay(new Way());
                clippedWay->id = way.id;
                setCoordinates(*clippedWay, (*it)->Contour);
                relation->elements.push_back(clippedWay);
            }
            callback(relation);
        }
    }

    void clipArea(const Area& area, const std::function<void(std::shared_ptr<Element>)>& callback)
    {
        ClipperLib::Path areaShape;
        PointLocation pointLocation = setPath(area, areaShape);
        // 1. all geometry inside current quadkey: no need to truncate.
        if (pointLocation == PointLocation::AllInside) {
            callback(std::shared_ptr<Area>(new Area(area)));
            return;
        }

        // 2. all geometry outside : pass empty
        if (pointLocation == PointLocation::AllOutside) {
            auto emptyArea = std::shared_ptr<Area>(new Area());
            emptyArea->id = area.id;
            emptyArea->tags = area.tags;
            callback(emptyArea);
            return;
        }

        ClipperLib::Paths solution;
        clipper_.AddPath(areaShape, ClipperLib::ptSubject, true);
        clipper_.AddPath(createPathFromBoundingBox(), ClipperLib::ptClip, true);
        clipper_.Execute(ClipperLib::ctIntersection, solution);
        clipper_.Clear();

        // 3. way intersects border only once: store a copy with clipped geometry
        if (solution.size() == 1) {
            auto clippedArea = std::shared_ptr<Area>(new Area());
            clippedArea->id = area.id;
            clippedArea->tags = area.tags;
            setCoordinates(*clippedArea, solution[0]);
            callback(clippedArea);
        }
        // 4. in this case, result should be stored as relation (collection of areas)
        else {
            auto relation = std::shared_ptr<Relation>(new Relation());
            relation->id = area.id;
            relation->tags = area.tags;
            relation->elements.reserve(solution.size());
            for (auto it = solution.begin(); it != solution.end(); ++it) {
                std::shared_ptr<Area> clippedArea(new Area());
                clippedArea->id = area.id;
                setCoordinates(*clippedArea, *it);
                relation->elements.push_back(clippedArea);
            }
            callback(relation);
        }
    }

private:
    // Defines polygon points location relative to current quadkey.
    enum PointLocation { AllInside, AllOutside, Mixed };

    template<typename T>
    PointLocation setPath(const T& t, ClipperLib::Path& shape) {
        shape.reserve(t.coordinates.size());
        bool allInside = false;
        bool allOutside = true;
        for (const GeoCoordinate& coord : t.coordinates) {
            bool contains = quadKeyBboxPtr->contains(coord);
            allInside &= contains;
            allOutside &= !contains;
            shape.push_back(ClipperLib::IntPoint(coord.longitude*Scale, coord.latitude*Scale));
        }

        return allInside ? PointLocation::AllInside :
            (allOutside ? PointLocation::AllOutside : PointLocation::Mixed);
    }

    template<typename T>
    void setCoordinates(T& t, const ClipperLib::Path& path) {
        t.coordinates.reserve(path.size());
        for (const auto& c : path) {
            t.coordinates.push_back(GeoCoordinate(c.Y / Scale, c.X / Scale));
        }
    }

    ClipperLib::Path createPathFromBoundingBox()
    {
        double xMin = quadKeyBboxPtr->minPoint.longitude, yMin = quadKeyBboxPtr->minPoint.latitude,
            xMax = quadKeyBboxPtr->maxPoint.longitude, yMax = quadKeyBboxPtr->maxPoint.latitude;
        ClipperLib::Path rect;
        rect.push_back(ClipperLib::IntPoint(xMin*Scale, yMin*Scale));
        rect.push_back(ClipperLib::IntPoint(xMax*Scale, yMin*Scale));
        rect.push_back(ClipperLib::IntPoint(xMax*Scale, yMax*Scale));
        rect.push_back(ClipperLib::IntPoint(xMin*Scale, yMax*Scale));
        return std::move(rect);
    }

    const double Scale = 1E8; // max precision for Lat/Lon
    const double DoubleScale = Scale * Scale;
    ClipperLib::Clipper clipper_;
};

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

    ElementGeometryVisitor(ElementStore& elementStore) :
        elementStore_(elementStore),
        quadKeyPtr_(nullptr),
        geometryClipper_()
    {
    }

    void clipAndStore(const Element& element, const QuadKey& quadKey, const BoundingBox& quadKeyBbox)
    {
        quadKeyPtr_ = &quadKey;
        geometryClipper_.quadKeyBboxPtr = &quadKeyBbox;
        element.accept(*this);
    }

private:

    void visitNode(const Node& node)
    {
        geometryClipper_.clipNode(node, [&](std::shared_ptr<Element> element) { elementStore_.storeImpl(*element, *quadKeyPtr_); });
    }

    void visitWay(const Way& way)
    {
        geometryClipper_.clipWay(way, [&](std::shared_ptr<Element>& element) { elementStore_.storeImpl(*element, *quadKeyPtr_); });
    }

    void visitArea(const Area& area)
    {
        geometryClipper_.clipArea(area, [&](std::shared_ptr<Element>& element) { elementStore_.storeImpl(*element, *quadKeyPtr_); });
    }

    void visitRelation(const Relation& relation)
    {
        struct RelationVisitor : public ElementVisitor
        {
            Relation data;
            ElementGeometryClipper& clipper;

            RelationVisitor(ElementGeometryClipper& geometryClipper) : clipper(geometryClipper) { }

            void visitNode(const Node& node) 
            { 
                clipper.clipNode(node, [&](std::shared_ptr<Element> element) { data.elements.push_back(element); });
            }

            void visitWay(const Way& way)
            {
                clipper.clipWay(way, [&](std::shared_ptr<Element> element) { data.elements.push_back(element); });
            }

            void visitArea(const Area& area)
            {
                clipper.clipArea(area, [&](std::shared_ptr<Element> element) { data.elements.push_back(element); });
            }

            void visitRelation(const Relation& relation)
            {
                for (const auto& element : relation.elements) { element->accept(*this); }
            }
        } visitor(geometryClipper_);

        relation.accept(visitor);
        elementStore_.storeImpl(visitor.data, *quadKeyPtr_);
    }

    ElementStore& elementStore_;
    ElementGeometryClipper geometryClipper_;
    const QuadKey* quadKeyPtr_;
};

ElementStore::ElementStore(const StyleProvider& styleProvider, StringTable& stringTable) :
    styleProvider_(styleProvider),
    stringTable_(stringTable)
{
}

ElementStore::~ElementStore()
{
}

bool ElementStore::store(const Element& element, const utymap::index::LodRange& range)
{
    BoundingBoxVisitor bboxVisitor;
    bool wasStored = false;
    uint32_t clipKeyId = stringTable_.getId("clip");
    for (int lod = range.start; lod <= range.end; ++lod) {
        // skip element for this lod
        Style style = styleProvider_.forElement(element, lod);
        if (!style.isApplicable)
            continue;
        // initialize bounding box only once
        if (!bboxVisitor.boundingBox.isValid()) {
            element.accept(bboxVisitor);
        }
        storeInTileRange(element, bboxVisitor.boundingBox, lod, style.has(clipKeyId));
        wasStored = true;
    }

    // NOTE still might be clipped and then skipped
    return wasStored;
}

void ElementStore::storeInTileRange(const Element& element, const BoundingBox& elementBbox, int levelOfDetails, bool shouldClip)
{
    ElementGeometryVisitor geometryClipper(*this);
    GeoUtils::visitTileRange(elementBbox, levelOfDetails, [&](const QuadKey& quadKey, const BoundingBox& quadKeyBbox) {
        if (shouldClip)
            geometryClipper.clipAndStore(element, quadKey, quadKeyBbox);
        else
            storeImpl(element, quadKey);
    });
}

}}
