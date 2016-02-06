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
#include "mapcss/StyleProvider.hpp"

#include <cstdint>
#include <memory>

namespace utymap { namespace index {

struct ShapeDataVisitor
{
    int nodes;
    int ways;
    int areas;
    int relations;

    ShapeDataVisitor(ElementStore& elementStore, 
                    const utymap::mapcss::StyleProvider& styleProvider, 
                    StringTable& stringTable, 
                    const LodRange& lodRange) :
        elementStore_(elementStore),
        styleProvider_(styleProvider),
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
        node.id = 0;
        node.coordinate = coordinate;
        setTags(node, tags);
        if (elementStore_.store(node, lodRange_, styleProvider_)) {
            nodes++;
        }
    }

    void visitWay(utymap::formats::Coordinates& coordinates, utymap::formats::Tags& tags, bool isRing)
    {
        if (isRing) {
            utymap::entities::Area area;
            area.id = 0;
            area.coordinates = std::move(coordinates);
            setTags(area, tags);
            if (elementStore_.store(area, lodRange_, styleProvider_))
                areas++;
        }
        else {
            utymap::entities::Way way;
            way.id = 0;
            way.coordinates = std::move(coordinates);
            setTags(way, tags);
            if (elementStore_.store(way, lodRange_, styleProvider_))
                ways++;
        }
    }

    void visitRelation(utymap::formats::PolygonMembers& members, utymap::formats::Tags& tags)
    {
        utymap::entities::Relation relation;
        relation.id = 0;
        setTags(relation, tags);
        for (const auto& member : members) {
            if (member.coordinates.size() == 1) {
                std::shared_ptr<utymap::entities::Node> node(new utymap::entities::Node());
                node->id = 0;
                node->coordinate = member.coordinates[0];
                relation.elements.push_back(node);
            }
            else if (member.isRing) {
                std::shared_ptr<utymap::entities::Area> area(new utymap::entities::Area());
                area->id = 0;
                area->coordinates = std::move(member.coordinates);
                relation.elements.push_back(area);
            }
            else {
                std::shared_ptr<utymap::entities::Way> way(new utymap::entities::Way());
                way->id = 0;
                way->coordinates = std::move(member.coordinates);
                relation.elements.push_back(way);
            }
        }
        if (elementStore_.store(relation, lodRange_, styleProvider_))
            relations++;
    }

private:
    ElementStore& elementStore_;
    const utymap::mapcss::StyleProvider& styleProvider_;
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
