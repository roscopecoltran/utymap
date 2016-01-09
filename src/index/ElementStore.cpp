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
        // TODO
    }
};

// Modifies geometry of element by bounding box clipping
class ElementGeometryClipper : private ElementVisitor
{
public:

    ElementGeometryClipper(ElementStore& elementStore) :
        elementStore_(elementStore),
        clipper_(),
        quadKey_(nullptr),
        quadKeyBbox_(nullptr)
    {
    }

    void clipAndStore(const Element& element, const QuadKey& quadKey, const BoundingBox& quadKeyBbox)
    {
        quadKey_ = &quadKey;
        quadKeyBbox_ = &quadKeyBbox;
        element.accept(*this);
    }

private:

    void visitNode(const Node& node)
    {
        if (quadKeyBbox_->contains(node.coordinate)) {
            elementStore_.storeImpl(node, *quadKey_);
        }
    }

    void visitWay(const Way& way)
    {
        ClipperLib::Path wayShape;
        bool shouldBeTruncated = setPath(way, wayShape);
        // 1. all geometry inside current quadkey: no need to truncate.
        if (!shouldBeTruncated) {
            elementStore_.storeImpl(way, *quadKey_);
            return;
        }

        ClipperLib::PolyTree solution;
        clipper_.AddPath(wayShape, ClipperLib::ptSubject, false);
        clipper_.AddPath(createPathFromBoundingBox(), ClipperLib::ptClip, true);
        clipper_.Execute(ClipperLib::ctIntersection, solution);
        clipper_.Clear();

        // 2. way intersects border only once: store a copy with clipped geometry
        if (solution.ChildCount() == 1) {
            ClipperLib::PolyNode* node = solution.Childs[0];
            Way clippedWay;
            clippedWay.id = way.id;
            clippedWay.tags = way.tags;
            setCoordinates(clippedWay, node->Contour);
            elementStore_.storeImpl(clippedWay, *quadKey_);
        }
        // 3. in this case, result should be stored as relation (collection of ways)
        else {
            Relation relation;
            relation.id = way.id;
            relation.tags = way.tags;
            relation.elements.reserve(solution.ChildCount());
            for (auto it = solution.Childs.begin(); it != solution.Childs.end(); ++it) {
                std::shared_ptr<Way> clippedWay(new Way());
                clippedWay->id = way.id;
                setCoordinates(*clippedWay, (*it)->Contour);
                relation.elements.push_back(clippedWay);
            }
            elementStore_.storeImpl(relation, *quadKey_);
        }
    }

    void visitArea(const Area& area)
    {
        ClipperLib::Path areaShape;
        bool shouldBeTruncated = setPath(area, areaShape);
        // 1. all geometry inside current quadkey: no need to truncate.
        if (!shouldBeTruncated) {
            elementStore_.storeImpl(area, *quadKey_);
            return;
        }

        ClipperLib::Paths solution;
        clipper_.AddPath(areaShape, ClipperLib::ptSubject, true);
        clipper_.AddPath(createPathFromBoundingBox(), ClipperLib::ptClip, true);
        clipper_.Execute(ClipperLib::ctIntersection, solution);
        clipper_.Clear();

        // 2. way intersects border only once: store a copy with clipped geometry
        if (solution.size() == 1) {
            Area clippedArea;
            clippedArea.id = area.id;
            clippedArea.tags = area.tags;
            setCoordinates(clippedArea, solution[0]);
            elementStore_.storeImpl(clippedArea, *quadKey_);
        }
        // 3. in this case, result should be stored as relation (collection of areas)
        else {
            Relation relation;
            relation.id = area.id;
            relation.tags = area.tags;
            relation.elements.reserve(solution.size());
            for (auto it = solution.begin(); it != solution.end(); ++it) {
                std::shared_ptr<Area> clippedArea(new Area());
                clippedArea->id = area.id;
                setCoordinates(*clippedArea, (*it));
                relation.elements.push_back(clippedArea);
            }
            elementStore_.storeImpl(relation, *quadKey_);
        }
    }

    void visitRelation(const Relation& relation)
    {
    }

    template<typename T>
    bool setPath(const T& t, ClipperLib::Path& shape) {
        shape.reserve(t.coordinates.size());
        bool shouldBeTruncated = false;
        for (const GeoCoordinate& coord : t.coordinates) {
            shouldBeTruncated |= !quadKeyBbox_->contains(coord);
            shape.push_back(ClipperLib::IntPoint(coord.longitude*Scale, coord.latitude*Scale));
        }

        return shouldBeTruncated;
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
        double xMin = quadKeyBbox_->minPoint.longitude, yMin = quadKeyBbox_->minPoint.latitude,
            xMax = quadKeyBbox_->maxPoint.longitude, yMax = quadKeyBbox_->maxPoint.latitude;
        ClipperLib::Path rect;
        rect.push_back(ClipperLib::IntPoint(xMin*Scale, yMin*Scale));
        rect.push_back(ClipperLib::IntPoint(xMax*Scale, yMin*Scale));
        rect.push_back(ClipperLib::IntPoint(xMax*Scale, yMax*Scale));
        rect.push_back(ClipperLib::IntPoint(xMin*Scale, yMax*Scale));
        return std::move(rect);
    }

    const double Scale = 1E8; // max precision for Lat/Lon
    const double DoubleScale = Scale * Scale;

    ElementStore& elementStore_;
    ClipperLib::Clipper clipper_;
    const QuadKey* quadKey_;
    const BoundingBox* quadKeyBbox_;
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
    ElementGeometryClipper geometryClipper(*this);
    GeoUtils::visitTileRange(elementBbox, levelOfDetails, [&](const QuadKey& quadKey, const BoundingBox& quadKeyBbox) {
        if (shouldClip)
            geometryClipper.clipAndStore(element, quadKey, quadKeyBbox);
        else
            storeImpl(element, quadKey);
    });
}

}}
