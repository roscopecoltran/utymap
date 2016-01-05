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

using namespace utymap;
using namespace utymap::index;
using namespace utymap::entities;
using namespace utymap::formats;

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
struct GeometryVisitor : public ElementVisitor
{
    const BoundingBox& boundingBox;
    bool isInside;

    GeometryVisitor(const BoundingBox& bbox) :
        boundingBox(bbox),
        isInside(false)
    {
    }

    void visitNode(const Node& node)
    {
    }

    void visitWay(const Way& way)
    {
    }

    void visitArea(const Area& area)
    {
    }

    void visitRelation(const Relation& relation)
    {
    }
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
        if (!styleFilter_.isApplicable(element, lod))
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
    auto tileRangeVisitor = [&](const QuadKey& quadKey, const BoundingBox& quadKeyBbox) {
        GeometryVisitor geometryVisitor(quadKeyBbox);
        element.accept(geometryVisitor);
        if (geometryVisitor.isInside) {
            store(element, quadKey);
        }
    };
    GeoUtils::visitTileRange(elementBbox, levelOfDetails, tileRangeVisitor);
}
