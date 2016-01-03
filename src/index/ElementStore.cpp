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

void ElementStore::store(const GeoPolygon& polygon,
    const utymap::formats::Tags& tags,
    const ElementStore::ElementType elementType)
{
    StringTable& stringTable = getStringTable();
    const StyleFilter& styleFilter = getStyleFilter();
    ElementVisitor& elementVisitor = getElementVisitor();
    
    BoundingBox bbox;
    Element* element = createElement(elementType);
    for (int lod = MinLevelOfDetails; lod <= MaxLevelOfDetails; ++lod) {
        // skip element for this lod
        if (!styleFilter.isApplicable(*element, lod))
            continue;

        // initialize tags only once
        if (element->tags.size() == 0 && tags.size() > 0) {
            element->tags.reserve(tags.size());
            for (const auto& tag : tags) {
                uint32_t key = stringTable.getId(tag.key);
                uint32_t value = stringTable.getId(tag.value);
                element->tags.push_back(utymap::entities::Tag(key, value));
            }
            stringTable.flush();
        }
        // initialize bounding box only once
        if (!bbox.isValid()) {
            for (const auto& shape : polygon) {
                for (const GeoCoordinate& c : shape) {
                    bbox.expand(c);
                }
            }
        }

        storeInTileRange(*element, bbox, lod, elementVisitor);
    }

    delete element;
}

Element* ElementStore::createElement(const ElementStore::ElementType elementType) const
{
    switch (elementType)
    {
        case Node: return new utymap::entities::Node();
        case Way:  return new utymap::entities::Way();
        case Area: return new utymap::entities::Area();
        case Relation: return new utymap::entities::Relation();
    }
}

void ElementStore::storeInTileRange(Element& element, const BoundingBox& elementBbox, int levelOfDetails, ElementVisitor& elementVisitor)
{
    GeoUtils::visitTileRange(elementBbox, levelOfDetails,
        [&](const QuadKey& quadKey, const BoundingBox& quadKeyBbox) {

        // TODO
        element.accept(elementVisitor);
    });
}