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
#include "index/StyleFilter.hpp"
#include "index/GeoUtils.hpp"
#include "meshing/clipper.hpp"

using namespace utymap;
using namespace utymap::entities;
using namespace utymap::formats;

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

    void clip(const Element& element, const QuadKey& quadKey, const BoundingBox& quadKeyBbox)
    {
        quadKey_ = &quadKey;
        quadKeyBbox_ = &quadKeyBbox;
        element.accept(*this);
    }

private:

    void visitNode(const Node& node)
    {
        if (quadKeyBbox_->contains(node.coordinate)) {
            elementStore_.store(node, *quadKey_);
        }
    }

    void visitWay(const Way& way)
    {
        ClipperLib::Paths solution;
        ClipperLib::Path wayShape;
        wayShape.reserve(way.coordinates.size());
        bool shouldBeTruncated = false;
        for (const GeoCoordinate& coord : way.coordinates) {
            shouldBeTruncated |= !quadKeyBbox_->contains(coord);
            wayShape.push_back(ClipperLib::IntPoint(coord.longitude*Scale, coord.latitude*Scale));
        }

        // all geometry inside current quadkey: no need to truncate.
        if (!shouldBeTruncated) {
            elementStore_.store(way, *quadKey_);
            return;
        }

        clipper_.AddPath(wayShape, ClipperLib::ptSubject, false);
        clipper_.AddPath(createPathFromBoundingBox(), ClipperLib::ptClip, true);
        clipper_.Execute(ClipperLib::ctIntersection, solution);
        clipper_.Clear();

        // TODO it is possible that result should be stored as relation (collection of ways)

        /*Way clippedWay;
        clippedWay.id = way.id;
        clippedWay.tags = way.tags;
        clippedWay.coordinates.reserve(solution.size());

        for (const auto& point : solution) {
            clippedWay.coordinates.push_back(GeoCoordinate(point. / Scale, point.X / Scale))
        }*/
    }

    void visitArea(const Area& area)
    {
    }

    void visitRelation(const Relation& relation)
    {
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

ElementStore::ElementStore(const StyleFilter& styleFilter) :
     styleFilter_(styleFilter)
{
}

ElementStore::~ElementStore()
{
}

bool ElementStore::store(const Element& element)
{
    BoundingBoxVisitor bboxVisitor;
    bool wasStored = false;
    for (int lod = MinLevelOfDetails; lod <= MaxLevelOfDetails; ++lod) {
        // skip element for this lod
        Style style = styleFilter_.get(element, lod);
        if (!style.isApplicable)
            continue;
        // initialize bounding box only once
        if (!bboxVisitor.boundingBox.isValid()) {
            element.accept(bboxVisitor);
        }
        storeInTileRange(element, bboxVisitor.boundingBox, lod);
        wasStored = true;
    }

    // NOTE still might be clipped and then skipped
    return wasStored;
}

void ElementStore::storeInTileRange(const Element& element, const BoundingBox& elementBbox, int levelOfDetails)
{
    ElementGeometryClipper geometryClipper(*this);
    auto tileRangeVisitor = [&](const QuadKey& quadKey, const BoundingBox& quadKeyBbox) {
        geometryClipper.clip(element, quadKey, quadKeyBbox);
    };
    GeoUtils::visitTileRange(elementBbox, levelOfDetails, tileRangeVisitor);
}

}}