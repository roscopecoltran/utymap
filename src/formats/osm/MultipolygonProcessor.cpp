#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "formats/osm/MultipolygonProcessor.hpp"
#include "index/GeoUtils.hpp"

#include <algorithm>
#include <deque>

using namespace utymap;
using namespace utymap::entities;
using namespace utymap::formats;
using namespace utymap::index;

typedef std::deque<GeoCoordinate> Coords;
typedef std::vector<int> Ints;

struct MultipolygonProcessor::CoordinateSequence
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

    inline bool isClosed() 
    { 
        return coordinates.size() > 1 && coordinates[0] == coordinates[coordinates.size() - 1]; 
    }

    inline bool containsRing(const Coords& other) const
    {
        return std::all_of(other.begin(), other.end(), [&](const GeoCoordinate& c) {
            return utymap::index::GeoUtils::isPointInPolygon(c, coordinates.begin(), coordinates.end());
        });
    }

private:

    inline GeoCoordinate first() const { return coordinates[0]; }

    inline GeoCoordinate last() const { return coordinates[coordinates.size() - 1]; }

    inline void addToBegin(const Coords& other) { coordinates.insert(coordinates.begin(), other.begin(), other.end()); }

    inline void addToEnd(const Coords& other) { coordinates.insert(coordinates.end(), other.begin(), other.end()); }

    inline void reverse() { std::reverse(coordinates.begin(), coordinates.end()); }

    inline void mergeTags(const utymap::formats::Tags& other) { tags.insert(tags.end(), other.begin(), other.end()); }
};


MultipolygonProcessor::MultipolygonProcessor(std::uint64_t id, RelationMembers& members, Tags& tags, StringTable& stringTable,
    std::unordered_map<std::uint64_t, std::shared_ptr<utymap::entities::Area>>& areaMap,
    std::unordered_map<std::uint64_t, std::shared_ptr<utymap::entities::Way>>& wayMap) :
    id_(id), members_(members), tags_(tags), stringTable_(stringTable), areaMap_(areaMap), wayMap_(wayMap)
{
}

// see http://wiki.openstreetmap.org/wiki/Relation:multipolygon/Algorithm
Relation MultipolygonProcessor::process()
{
    Relation relation;
    bool allClosed = true;

    Ints outerIndecies;
    Ints innerIndecies;
    CoordinateSequences sequences;
    for (const auto& member : members_)
    {
        if (member.type != "way") continue;

        Coordinates coordinates;
        Tags tags;
        auto wayPair = wayMap_.find(member.refId);
        if (wayPair != wayMap_.end()) {
            coordinates = wayPair->second->coordinates;
            tags = restoreTags(wayPair->second->tags);
        }
        else {
            auto areaPair = areaMap_.find(member.refId);
            if (areaPair != areaMap_.end()) {
                coordinates = areaPair->second->coordinates;
                tags = restoreTags(areaPair->second->tags);
            }
        }

        if (coordinates.empty()) continue;

        if (member.role == "outer")
            outerIndecies.push_back(sequences.size());
        else if (member.role == "inner")
            innerIndecies.push_back(sequences.size());
        else
            continue;

        // TODO what should be used as Id?
        sequences.push_back(std::shared_ptr<CoordinateSequence>(new CoordinateSequence(id_, coordinates, tags)));
    }

    if (outerIndecies.size() == 1 && allClosed)
        simpleCase(relation, sequences, outerIndecies, innerIndecies);
    else
        complexCase(relation, sequences);

    return std::move(relation);
}

Tags MultipolygonProcessor::restoreTags(const std::vector<utymap::entities::Tag>& tags) const
{
    Tags convertedTags;
    std::transform(tags.begin(), tags.end(), convertedTags.begin(), [&](const utymap::entities::Tag& tag) {
        return utymap::formats::Tag{
            stringTable_.getString(tag.key),
            stringTable_.getString(tag.value)
        };
    });
    return std::move(convertedTags);
}

std::vector<utymap::entities::Tag> MultipolygonProcessor::convertTags(const Tags& tags) const
{
    std::vector<utymap::entities::Tag> convertedTags;
    std::transform(tags.begin(), tags.end(), convertedTags.begin(), [&](const utymap::formats::Tag& tag) {
        return utymap::entities::Tag{
            stringTable_.getId(tag.key),
            stringTable_.getId(tag.value)
        };
    });
    return std::move(convertedTags);
}

void MultipolygonProcessor::simpleCase(Relation& relation, const CoordinateSequences& sequences, const Ints& outerIndecies, const Ints& innerIndecies)
{
    // TODO set correct tags!
    auto outer = sequences[outerIndecies[0]];

    // TODO investigate case of empty tags
    auto tags = getTags(relation, *outer);
    if (tags.empty()) return;

    // outer
    std::shared_ptr<Area> outerArea(new Area());
    outerArea->id = outer->id;
    outerArea->tags = tags;
    outerArea->coordinates.insert(outerArea->coordinates.end(), outer->coordinates.begin(), outer->coordinates.end());
    relation.elements.push_back(outerArea);

    // inner
    for (int i : innerIndecies) {
        auto coords = sequences[i]->coordinates;
        std::shared_ptr<Area> innerArea(new Area());
        // TODO ensure hole orientation
        innerArea->coordinates.insert(innerArea->coordinates.end(), coords.begin(), coords.end());
        relation.elements.push_back(innerArea);
    }
}

void MultipolygonProcessor::complexCase(Relation& relation, CoordinateSequences& sequences)
{
    CoordinateSequences rings = createRings(sequences);
    if (rings.empty()) return;

    fillRelation(relation, rings);
}

std::vector<std::shared_ptr<MultipolygonProcessor::CoordinateSequence>> MultipolygonProcessor::createRings(CoordinateSequences& sequences)
{
    CoordinateSequences closedRings;
    std::shared_ptr<MultipolygonProcessor::CoordinateSequence> currentRing = nullptr;
    while (!sequences.empty()) {
        if (currentRing == nullptr) {
            // start a new ring with any remaining node sequence
            auto lastIndex = sequences.size() - 1;
            currentRing = sequences[lastIndex];
            sequences.erase(sequences.begin() + lastIndex);
        }
        else {
            // try to continue the ring by appending a node sequence
            bool isFound = false;

            for (auto it = sequences.begin(); it != sequences.end(); ++it) {
                if (!currentRing->tryAdd(**it)) continue;
                sequences.erase(it);
                break;
            }

            if (!isFound) return CoordinateSequences();
        }

        // check whether the ring under construction is closed
        if (currentRing != nullptr && currentRing->isClosed())
        {
            // TODO check that it isn't self-intersecting!
            closedRings.push_back(std::shared_ptr<CoordinateSequence>(new CoordinateSequence(*currentRing)));
            currentRing = nullptr;
        }

        return currentRing != nullptr ? CoordinateSequences() : std::move(closedRings);
    }

    return std::move(closedRings);
}

void MultipolygonProcessor::fillRelation(Relation& relation, CoordinateSequences& rings)
{
    while (!rings.empty()) {
        // find an outer ring
        std::shared_ptr<CoordinateSequence> outer = nullptr;
        for (const auto& candidate : rings) {
            bool containedInOtherRings = false;
            for (const auto& other : rings) {
                if (&other != &candidate && other->containsRing(candidate->coordinates)) {
                    containedInOtherRings = true;
                    break;
                }
            }
            if (containedInOtherRings) continue;
            outer = candidate;
            break;
        }

        // find inner rings of that ring
        CoordinateSequences inners;
        for (const auto& ring : rings) {
            if (&ring != &outer && outer->containsRing(ring->coordinates)) {
                bool containedInOthers = false;
                for (const auto& other : rings) {
                    if (&other != &ring && &other != &outer && other->containsRing(ring->coordinates))
                    {
                        containedInOthers = true;
                        break;
                    }
                }
                if (!containedInOthers)
                    inners.push_back(ring);
            }
        }

        auto tags = getTags(relation, *outer);
        // TODO investigate case of null/empty tags
        if (tags.empty()) return;

        // inner: create a new area and remove the used rings
        for (const auto& innerRing : inners) {
            // TODO ensure hole orientation
            std::shared_ptr<Area> innerArea(new Area());
            innerArea->tags = convertTags(innerRing->tags);
            innerArea->coordinates.insert(innerArea->coordinates.end(), innerRing->coordinates.begin(), innerRing->coordinates.end());
            relation.elements.push_back(innerArea);
        }

        // outer
        std::shared_ptr<Area> outerArea(new Area());
        outerArea->id = outer->id;
        outerArea->tags = std::move(tags);
        outerArea->coordinates.insert(outerArea->coordinates.end(), outer->coordinates.begin(), outer->coordinates.end());
        relation.elements.push_back(outerArea);
    }
}

std::vector<utymap::entities::Tag> MultipolygonProcessor::getTags(const Relation& relation, const CoordinateSequence& outer) const
{
    // TODO tag processing
    return relation.tags.size() > 1 ? relation.tags : convertTags(outer.tags);
}
