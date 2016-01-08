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
        wayShape.reserve(way.coordinates.size());
        bool shouldBeTruncated = false;
        for (const GeoCoordinate& coord : way.coordinates) {
            shouldBeTruncated |= !quadKeyBbox_->contains(coord);
            wayShape.push_back(ClipperLib::IntPoint(coord.longitude*Scale, coord.latitude*Scale));
        }

        // all geometry inside current quadkey: no need to truncate.
        if (!shouldBeTruncated) {
            elementStore_.storeImpl(way, *quadKey_);
            return;
        }

        ClipperLib::PolyTree solution;
        clipper_.AddPath(wayShape, ClipperLib::ptSubject, false);
        clipper_.AddPath(createPathFromBoundingBox(), ClipperLib::ptClip, true);
        clipper_.Execute(ClipperLib::ctIntersection, solution);
        clipper_.Clear();

        // way intersects border only once: store a copy with clipped geometry
        if (solution.ChildCount() == 1) {
            ClipperLib::PolyNode* node = solution.Childs[0];
            Way clippedWay;
            clippedWay.id = way.id;
            clippedWay.tags = way.tags;
            clippedWay.coordinates.reserve(node->Contour.size());
            for (const auto& c : node->Contour) {
                clippedWay.coordinates.push_back(GeoCoordinate(c.Y / Scale, c.X / Scale));
            }
            elementStore_.storeImpl(clippedWay, *quadKey_);
        }
        else {
            // TODO in this case, result should be stored as relation (collection of ways)
        }
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

ElementStore::ElementStore(const StyleProvider& styleProvider, StringTable& stringTable) :
    styleProvider_(styleProvider),
    stringTable_(stringTable)
{
}

ElementStore::~ElementStore()
{
}

bool ElementStore::store(const Element& element)
{
    BoundingBoxVisitor bboxVisitor;
    bool wasStored = false;
    uint32_t clipKeyId = stringTable_.getId("clip");
    for (int lod = GeoUtils::MinLevelOfDetails; lod <= GeoUtils::MaxLevelOfDetails; ++lod) {
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
