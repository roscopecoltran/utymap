#include "BoundingBox.hpp"
#include "QuadKey.hpp"
#include "entities/Element.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "formats/FormatTypes.hpp"
#include "index/ElementGeometryClipper.hpp"

using namespace utymap;
using namespace utymap::entities;
using namespace utymap::formats;
using namespace utymap::mapcss;

namespace {
    const static std::string ClipKey = "clip";
    const static std::string SkipKey = "skip";

    // Creates bounding box of given element.
    class BoundingBoxVisitor : public ElementVisitor
    {
    public:
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
}

namespace utymap { namespace index {

ElementStore::ElementStore(StringTable& stringTable) :
    clipKeyId_(stringTable.getId(ClipKey)), skipKeyId_(stringTable.getId(SkipKey))
{
}

ElementStore::~ElementStore()
{
}

bool ElementStore::store(const Element& element, const utymap::LodRange& range, const StyleProvider& styleProvider)
{
    return store(element, range, styleProvider, [&](const BoundingBox&, const BoundingBox&) {
        return true;
    });
}

bool ElementStore::store(const Element& element, const QuadKey& quadKey, const StyleProvider& styleProvider)
{
    const BoundingBox expectedQuadKeyBbox = utymap::utils::GeoUtils::quadKeyToBoundingBox(quadKey);
    return store(element, 
                LodRange(quadKey.levelOfDetail, quadKey.levelOfDetail), 
                styleProvider, 
                [&](const BoundingBox& elementBoundingBox, const BoundingBox& quadKeyBbox) {
                    return elementBoundingBox.intersects(expectedQuadKeyBbox) &&
                           expectedQuadKeyBbox.center() == quadKeyBbox.center();
                });
}

bool ElementStore::store(const Element& element, const BoundingBox& bbox, const utymap::LodRange& range, const StyleProvider& styleProvider)
{
    return store(element, range, styleProvider, [&](const BoundingBox& elementBoundingBox, const BoundingBox& quadKeyBbox) {
        return elementBoundingBox.intersects(bbox);
    });
}

template <typename Visitor>
bool ElementStore::store(const Element& element, const LodRange& range, const StyleProvider& styleProvider, const Visitor& visitor)
{
    BoundingBoxVisitor bboxVisitor;
    using namespace std::placeholders;
    ElementGeometryClipper geometryClipper(std::bind(&ElementStore::storeImpl, this, _1, _2));
    bool wasStored = false;
    for (int lod = range.start; lod <= range.end; ++lod) {
        if (!styleProvider.hasStyle(element, lod))
            continue;
        Style style = styleProvider.forElement(element, lod);
        if (style.has(skipKeyId_, "true")) continue;

        // initialize bounding box only once
        if (!bboxVisitor.boundingBox.isValid())
            element.accept(bboxVisitor);

         utymap::utils::GeoUtils::visitTileRange(bboxVisitor.boundingBox, lod, [&](const QuadKey& quadKey, const BoundingBox& quadKeyBbox) {
             if (!visitor(bboxVisitor.boundingBox, quadKeyBbox)) 
                 return false;

            if (style.has(clipKeyId_, "true"))
                geometryClipper.clipAndCall(element, quadKey, quadKeyBbox);
            else
                storeImpl(element, quadKey);
        });
        wasStored = true;
    }

    // NOTE still might be clipped and then skipped
    return wasStored;
}

}}
