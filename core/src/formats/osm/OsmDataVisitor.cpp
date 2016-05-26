#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "formats/osm/BuildingProcessor.hpp"
#include "formats/osm/MultipolygonProcessor.hpp"
#include "formats/osm/RelationProcessor.hpp"
#include "formats/osm/OsmDataVisitor.hpp"
#include "utils/ElementUtils.hpp"
#include "utils/GeometryUtils.hpp"

#include <algorithm>
#include <vector>

using namespace utymap;
using namespace utymap::formats;
using namespace utymap::entities;
using namespace utymap::index;

namespace {

    std::unordered_set<std::string> AreaKeys
    {
        "building",
        "building:part",
        "landuse",
        "amenity",
        "harbour",
        "historic",
        "leisure",
        "man_made",
        "military",
        "natural",
        "office",
        "place",
        "power",
        "public_transport",
        "shop",
        "sport",
        "tourism",
        "waterway",
        "wetland",
        "water",
        "aeroway",
        "addr:housenumber",
        "addr:housename"
    };

    std::unordered_set<std::string> FalseKeys
    {
        "no", "No", "NO", "false", "False", "FALSE", "0"
    };
}


void OsmDataVisitor::visitBounds(BoundingBox bbox)
{
}

void OsmDataVisitor::visitNode(std::uint64_t id, GeoCoordinate& coordinate, utymap::formats::Tags& tags)
{
    std::shared_ptr<Node> node(new Node());
    node->id = id;
    node->coordinate = coordinate;
    utymap::utils::setTags(stringTable_, *node, tags);
    context_.nodeMap[id] = node;
}

void OsmDataVisitor::visitWay(std::uint64_t id, std::vector<std::uint64_t>& nodeIds, utymap::formats::Tags& tags)
{
    std::vector<GeoCoordinate> coordinates;
    coordinates.reserve(nodeIds.size());
    for (auto nodeId : nodeIds) {
        coordinates.push_back(context_.nodeMap[nodeId]->coordinate);
    }

    if (coordinates.size() > 2 && isArea(tags)) {
        coordinates.pop_back();
        std::shared_ptr<Area> area(new Area());
        area->id = id;
        if (!utymap::utils::isClockwise(coordinates)) {
            std::reverse(coordinates.begin(), coordinates.end());
        }
        area->coordinates = std::move(coordinates);
        utymap::utils::setTags(stringTable_, *area, tags);
        context_.areaMap[id] = area;

    } else {
        std::shared_ptr<Way> way(new Way());
        way->id = id;
        way->coordinates = std::move(coordinates);
        utymap::utils::setTags(stringTable_, *way, tags);
        context_.wayMap[id] = way;
    }
}

void OsmDataVisitor::visitRelation(std::uint64_t id, RelationMembers& members, utymap::formats::Tags& tags)
{
    if (hasTag("type", "multipolygon", tags)) {
        MultipolygonProcessor processor(id, members, tags, stringTable_, context_);
        context_.relationMap[id] = processor.process();
    }
    else if (hasTag("type", "building", tags)) {
        BuildingProcessor processor(id, members, tags, stringTable_, context_);
        context_.relationMap[id] = processor.process();
    }
    else {
        RelationProcessor processor(id, members, tags, stringTable_, context_);
        context_.relationMap[id] = processor.process();
    }
}

bool OsmDataVisitor::isArea(const utymap::formats::Tags& tags) const
{
    for (const auto& tag : tags) {
        if (AreaKeys.find(tag.key) != AreaKeys.end() &&
            FalseKeys.find(tag.value) == FalseKeys.end())
            return true;
    }

    return false;
}

bool OsmDataVisitor::hasTag(const std::string& key, const std::string& value, const utymap::formats::Tags& tags) const
{
    for (const auto& tag : tags) {
        if (tag.key == key && tag.value == value)
            return true;
    }
    return false;
}

void OsmDataVisitor::complete()
{
    for (const auto& pair : context_.nodeMap) {
        add_(*pair.second);
    }

    for (const auto& pair : context_.wayMap) {
        add_(*pair.second);
    }

    for (const auto& pair : context_.areaMap) {
        add_(*pair.second);
    }

    for (const auto& pair : context_.relationMap) {
        add_(*pair.second);
    }
}

OsmDataVisitor::OsmDataVisitor(StringTable& stringTable, std::function<bool(utymap::entities::Element&)> add) :
stringTable_(stringTable), add_(add), context_()
{
}
