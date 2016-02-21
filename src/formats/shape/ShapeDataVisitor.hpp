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
#include "utils/ElementUtils.hpp"

#include <algorithm>
#include <cstdint>
#include <memory>

namespace utymap { namespace formats {

struct ShapeDataVisitor
{
    int nodes;
    int ways;
    int areas;
    int relations;

    ShapeDataVisitor(utymap::index::ElementStore& elementStore,
                    const utymap::mapcss::StyleProvider& styleProvider,
                    utymap::index::StringTable& stringTable,
                    const utymap::index::LodRange& lodRange) :
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
        utymap::utils::setTags(stringTable_, node, tags);
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
            utymap::utils::setTags(stringTable_, area, tags);
            if (elementStore_.store(area, lodRange_, styleProvider_))
                areas++;
        }
        else {
            utymap::entities::Way way;
            way.id = 0;
            way.coordinates = std::move(coordinates);
            utymap::utils::setTags(stringTable_, way, tags);
            if (elementStore_.store(way, lodRange_, styleProvider_))
                ways++;
        }
    }

    void visitRelation(utymap::formats::PolygonMembers& members, utymap::formats::Tags& tags)
    {
        utymap::entities::Relation relation;
        relation.id = 0;
        utymap::utils::setTags(stringTable_, relation, tags);
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
    utymap::index::ElementStore& elementStore_;
    const utymap::mapcss::StyleProvider& styleProvider_;
    utymap::index::StringTable& stringTable_;
    const utymap::index::LodRange& lodRange_;
};

}}

#endif // INDEX_SHAPEDATAVISITOR_HPP_DEFINED
