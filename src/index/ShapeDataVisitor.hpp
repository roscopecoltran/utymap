#ifndef INDEX_SHAPEDATAVISITOR_HPP_DEFINED
#define INDEX_SHAPEDATAVISITOR_HPP_DEFINED

#include "BoundingBox.hpp"
#include "GeoCoordinate.hpp"
#include "entities/Element.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "formats/FormatTypes.hpp"
#include "index/ElementStore.hpp"
#include "index/LodRange.hpp"
#include "index/StringTable.hpp"

#include <cstdint>

namespace utymap { namespace index {

struct ShapeDataVisitor
{
    int nodes;
    int ways;
    int areas;
    int relations;

    ShapeDataVisitor(ElementStore& elementStore, StringTable& stringTable, const LodRange& lodRange) :
        elementStore_(elementStore),
        stringTable_(stringTable),
        lodRange_(lodRange),
        nodes(0),
        ways(0),
        areas(0),
        relations(0)
    {
    }

    void visitNode(utymap::GeoCoordinate& coordinate, utymap::formats::Tags& tags)
    {
        utymap::entities::Node node;
        node.coordinate = coordinate;
        setTags(node, tags);
        if (elementStore_.store(node, lodRange_)) {
            nodes++;
        }
    }

    void visitWay(utymap::formats::Coordinates& coordinates, utymap::formats::Tags& tags, bool isRing)
    {
        if (isRing) {
            utymap::entities::Area area;
            area.coordinates = coordinates;
            setTags(area, tags);
            if (elementStore_.store(area, lodRange_))
                areas++;
        }
        else {
            utymap::entities::Way way;
            way.coordinates = coordinates;
            setTags(way, tags);
            if (elementStore_.store(way, lodRange_))
                ways++;
        }
    }

    void visitRelation(utymap::formats::PolygonMembers& members, utymap::formats::Tags& tags)
    {
        /*
         GeoPolygon polygon;
            polygon.reserve(1);
            polygon.push_back(coordinates);
        */
        // TODO
        relations++;
    }

private:
    ElementStore& elementStore_;
    StringTable& stringTable_;
    const LodRange& lodRange_;

    void setTags(utymap::entities::Element& element, const utymap::formats::Tags& tags)
    {
        for (const auto& tag : tags) {
            uint32_t key = stringTable_.getId(tag.key);
            uint32_t value = stringTable_.getId(tag.value);
            element.tags.push_back(utymap::entities::Tag(key, value));
        }
        stringTable_.flush();
    }
};

}}

#endif // INDEX_SHAPEDATAVISITOR_HPP_DEFINED
