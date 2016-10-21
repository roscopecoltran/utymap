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
    const std::string ClipKey = "clip";
    const std::string SkipKey = "skip";
    const std::string SizeKey = "size";

    /// Creates bounding box of given element.
    class BoundingBoxVisitor : public ElementVisitor
    {
    public:
        BoundingBox boundingBox;

        void visitNode(const Node& node) override
        {
            boundingBox.expand(node.coordinate);
        }

        void visitWay(const Way& way) override
        {
            boundingBox.expand(way.coordinates.cbegin(), way.coordinates.cend());
        }

        void visitArea(const Area& area) override
        {
            boundingBox.expand(area.coordinates.cbegin(), area.coordinates.cend());
        }

        void visitRelation(const Relation& relation) override
        {
            for (const auto& element: relation.elements) {
                element->accept(*this);
            }
        }
    };

    bool checkSize(const utymap::BoundingBox& quadKeyBBox, const utymap::BoundingBox& elementBbox, double minSize) {
        return elementBbox.width() / quadKeyBBox.width() > minSize;
    }
}

namespace utymap { namespace index {

ElementStore::ElementStore(StringTable& stringTable) :
    clipKeyId_(stringTable.getId(ClipKey)),
    skipKeyId_(stringTable.getId(SkipKey)),
    sizeKeyId_(stringTable.getId(SizeKey))
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
    double size = -1; // match all by default
    for (int lod = range.start; lod <= range.end; ++lod) {
        if (!styleProvider.hasStyle(element, lod))
            continue;
        Style style = styleProvider.forElement(element, lod);
        if (style.empty() || style.has(skipKeyId_, "true")) 
            continue;

        // initialize bounding box and size only once
        if (!bboxVisitor.boundingBox.isValid()) {
            element.accept(bboxVisitor);
            // read size if present
            if (style.has(sizeKeyId_))
                size = style.getValue(sizeKeyId_, 1, bboxVisitor.boundingBox.center());
        }

         utymap::utils::GeoUtils::visitTileRange(bboxVisitor.boundingBox, lod,
                                                 [&](const QuadKey& quadKey, const BoundingBox& quadKeyBbox) {
             if (!visitor(bboxVisitor.boundingBox, quadKeyBbox) ||
                 !checkSize(quadKeyBbox, bboxVisitor.boundingBox, size)) // can be optimized (quadkey widht is const for lod)
                 return;

            if (style.has(clipKeyId_, "true"))
                geometryClipper.clipAndCall(element, quadKey, quadKeyBbox);
            else
                storeImpl(element, quadKey);

            wasStored = true;
        });

    }

    // NOTE still might be clipped and then skipped
    return wasStored;
}

}}
