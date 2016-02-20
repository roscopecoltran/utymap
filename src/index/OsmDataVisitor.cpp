#include "index/OsmDataVisitor.hpp"
#include "index/GeoUtils.hpp"

#include <algorithm>
#include <deque>
#include <vector>

using namespace utymap;
using namespace utymap::formats;
using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::mapcss;

std::unordered_set<std::string> OsmDataVisitor::AreaKeys
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

std::unordered_set<std::string> OsmDataVisitor::FalseKeys
{
    "no", "No", "NO", "false", "False", "FALSE", "0"
};

typedef std::deque<GeoCoordinate> Coords;

// Builds relation from multipolygon (see http://wiki.openstreetmap.org/wiki/Talk:Relation:multipolygon)
struct MultipolygonBuilder
{
private:
    // Helper class which provides the way to handle coordinate sequences.
    struct CoordinateSequence
    {
        std::uint64_t id;
        Coords coordinates;
        utymap::formats::Tags tags;

        CoordinateSequence(std::uint64_t id, const Coordinates& coordinates, utymap::formats::Tags& tags) :
            id(id), coordinates(coordinates.begin(), coordinates.end()), tags(tags)
        {
        }

        // Tries to add another sequence onto the start or end of this one.
        // If it succeeds, the other sequence may also be modified and
        // should be considered "spent".
        bool tryAdd(CoordinateSequence& other)
        {
            //add the sequence at the end
            if (last() == other.first()) {
                coordinates.pop_back();
                addToEnd(other.coordinates);
                mergeTags(other.tags);
                return true;
            }
            //add the sequence backwards at the end
            if (last() == other.last()) {
                coordinates.pop_back();
                other.reverse();
                mergeTags(other.tags);
                return true;
            }
            //add the sequence at the beginning
            if (first() == other.last()) {
                coordinates.pop_front();
                addToBegin(other.coordinates);
                mergeTags(other.tags);
                return true;
            }
            //add the sequence backwards at the beginning
            if (first() == other.first()) {
                coordinates.pop_front();
                other.reverse();
                addToBegin(other.coordinates);
                mergeTags(other.tags);
                return true;
            }
            return false;
        }

    private:

        inline GeoCoordinate first() const { return coordinates[0]; }

        inline GeoCoordinate last() const { return coordinates[coordinates.size() - 1]; }

        inline void addToBegin(const Coords& other) { coordinates.insert(coordinates.begin(), other.begin(), other.end()); }

        inline void addToEnd(const Coords& other) { coordinates.insert(coordinates.end(), other.begin(), other.end()); }

        inline void reverse() { std::reverse(coordinates.begin(), coordinates.end()); }

        inline void mergeTags(const utymap::formats::Tags& other) { tags.insert(tags.end(), other.begin(), other.end()); }

        inline bool isClosed() { return coordinates.size() > 1 && coordinates[0] == coordinates[coordinates.size() - 1]; }

        inline bool containsRing(const Coords& other) const
        {
            return std::all_of(other.begin(), other.end(), [&](const GeoCoordinate& c) {
                return utymap::index::GeoUtils::isPointInPolygon(c, coordinates.begin(), coordinates.end());
            });
        }
    };

public:
    MultipolygonBuilder(std::uint64_t id, RelationMembers& members, utymap::formats::Tags& tags):
        id_(id), members_(members), tags_(tags)
    {
    }

    Relation build()
    {
        Relation relation;

        return std::move(relation);
    }

private:
    std::uint64_t id_;
    RelationMembers& members_;
    utymap::formats::Tags& tags_;
};

void OsmDataVisitor::visitBounds(BoundingBox bbox)
{
    statistics.bounds++;
}

void OsmDataVisitor::visitNode(std::uint64_t id, GeoCoordinate& coordinate, utymap::formats::Tags& tags)
{
    nodeMap_[id] = coordinate;

    std::shared_ptr<Node> node(new Node());
    node->id = id;
    node->coordinate = coordinate;
    utymap::utils::setTags(stringTable_, *node, tags);

    if (elementStore_.store(*node, lodRange_, styleProvider_))
        statistics.nodes++;
    else
        statistics.skipNodes++;
}

void OsmDataVisitor::visitWay(std::uint64_t id, std::vector<std::uint64_t>& nodeIds, utymap::formats::Tags& tags)
{
    std::vector<GeoCoordinate> coordinates;
    coordinates.reserve(nodeIds.size());
    for (auto nodeId : nodeIds) {
        coordinates.push_back(nodeMap_[nodeId]);
    }
    wayMap_[id] = coordinates;

    if (coordinates.size() > 2 && isArea(tags)) {
        std::shared_ptr<Area> area(new Area());
        area->id = id;
        area->coordinates = coordinates;
        utymap::utils::setTags(stringTable_, *area, tags);
        if (elementStore_.store(*area, lodRange_, styleProvider_))
            statistics.areas++;
        return;
    } 

    std::shared_ptr<Way> way(new Way());
    way->id = id;
    way->coordinates = coordinates;
    utymap::utils::setTags(stringTable_, *way, tags);

    if (elementStore_.store(*way, lodRange_, styleProvider_))
        statistics.ways++;
    else
        statistics.skipWays++;
}

void OsmDataVisitor::visitRelation(std::uint64_t id, RelationMembers& members, utymap::formats::Tags& tags)
{
    if (hasTag("type", "multipolygon", tags)) {
        MultipolygonBuilder builder(id, members, tags);
        Relation relation = builder.build();
        if (!elementStore_.store(relation, lodRange_, styleProvider_)) {
            statistics.skipRelations++;
            return;
        }
    }
    else {
        std::vector<std::shared_ptr<Element>> elements;
        elements.reserve(members.size());
        for (const auto& member : members) {
            if (member.type == "node") {
                std::shared_ptr<Node> node(new Node());
                node->coordinate = nodeMap_[member.refId];
                elements.push_back(node);
            }
            else if (member.type == "way") {
                std::shared_ptr<Way> way(new Way());
                way->coordinates = wayMap_[member.refId];
                elements.push_back(way);
            }
            else {
                // not supported so far.
                statistics.skipRelations++;
                return;
            }
        }
    }

    statistics.relations++;
}

bool OsmDataVisitor::isArea(const utymap::formats::Tags& tags)
{
    for (const auto& tag : tags) {
        if (AreaKeys.find(tag.key) != AreaKeys.end() &&
            FalseKeys.find(tag.value) == FalseKeys.end())
            return true;
    }

    return false;
}

bool OsmDataVisitor::hasTag(const std::string& key, const std::string& value, const utymap::formats::Tags& tags)
{
    for (const auto& tag : tags) {
        if (tag.key == key && tag.value == value)
            return true;
    }
    return false;
}

OsmDataVisitor::OsmDataVisitor(ElementStore& elementStore, const StyleProvider& styleProvider,
                               StringTable& stringTable, const LodRange& lodRange) :
    elementStore_(elementStore), styleProvider_(styleProvider), 
    stringTable_(stringTable), lodRange_(lodRange), statistics()
{
}