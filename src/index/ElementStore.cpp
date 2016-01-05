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

ElementStore::~ElementStore()
{
}

// Creates bounding box of given element.
struct ElementGeometryVisitor : public ElementVisitor
{
    BoundingBox boundingBox;

    void visitNode(const Node& node) { boundingBox.expand(node.coordinate); }

    void visitWay(const Way& way) { boundingBox.expand(way.coordinates.cbegin(), way.coordinates.cend()); }

    void visitArea(const Area& area) { boundingBox.expand(area.coordinates.cbegin(), area.coordinates.cend()); }

    void visitRelation(const Relation& relation)
    {
        // TODO
    }
};

bool ElementStore::store(const utymap::entities::Element& element)
{
    const StyleFilter& styleFilter = getStyleFilter();

    ElementGeometryVisitor geometryVisitor;
    bool wasStored = false;
    for (int lod = MinLevelOfDetails; lod <= MaxLevelOfDetails; ++lod) {
        // skip element for this lod
        if (!styleFilter.isApplicable(element, lod))
            continue;
        // initialize bounding box only once
        if (!geometryVisitor.boundingBox.isValid()) {
            element.accept(geometryVisitor);
        }
        storeInTileRange(element, geometryVisitor.boundingBox, lod);
        wasStored = true;
    }

    // NOTE still might be clipped and then skipped
    return wasStored;
}

void ElementStore::storeInTileRange(const Element& element, const BoundingBox& elementBbox, int levelOfDetails)
{
    auto visitor = [&](const QuadKey& quadKey, const BoundingBox& quadKeyBbox) {
        // TODO use clipper to clip polygon by quadkey bounding box
        store(element, quadKey);
    };
    GeoUtils::visitTileRange(elementBbox, levelOfDetails, visitor);
}
