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

bool ElementStore::store(const GeoPolygon& polygon,
    const utymap::formats::Tags& tags,
    const ElementStore::ElementType elementType)
{
    StringTable& stringTable = getStringTable();
    const StyleFilter& styleFilter = getStyleFilter();

    Element* element = createElement(elementType, tags);
    BoundingBox bbox;
    bool wasStored = false;
    for (int lod = MinLevelOfDetails; lod <= MaxLevelOfDetails; ++lod) {
        // skip element for this lod
        if (!styleFilter.isApplicable(*element, lod))
            continue;
        // initialize bounding box only once
        if (!bbox.isValid()) {
            for (const auto& shape : polygon) {
                for (const GeoCoordinate& c : shape) {
                    bbox.expand(c);
                }
            }
        }
        storeInTileRange(*element, bbox, lod);
        wasStored = true;
    }

    delete element;
    return wasStored;
}

Element* ElementStore::createElement(const ElementStore::ElementType elementType,
                                     const utymap::formats::Tags& tags) const
{
    Element* element = nullptr;
    switch (elementType)
    {
        case Node: element = new utymap::entities::Node(); break;
        case Way:  element = new utymap::entities::Way(); break;
        case Area: element = new utymap::entities::Area(); break;
        case Relation: element = new utymap::entities::Relation(); break;
    }
    // convert tag colection
    StringTable& stringTable = getStringTable();
    element->tags.reserve(tags.size());
    for (const auto& tag : tags) {
        uint32_t key = stringTable.getId(tag.key);
        uint32_t value = stringTable.getId(tag.value);
        element->tags.push_back(utymap::entities::Tag(key, value));
    }
    stringTable.flush();

    return element;
}

void ElementStore::storeInTileRange(Element& element, const BoundingBox& elementBbox, int levelOfDetails)
{
    ElementVisitor& elementVisitor = getElementVisitor();
    auto visitor = [&](const QuadKey& quadKey, const BoundingBox& quadKeyBbox) {
        // TODO use clipper to clip polygon by quadkey bounding box
        element.accept(elementVisitor);
    };
    GeoUtils::visitTileRange(elementBbox, levelOfDetails, visitor);
}
