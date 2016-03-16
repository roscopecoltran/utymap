#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "formats/osm/MultipolygonProcessor.hpp"
#include "utils/GeoUtils.hpp"

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
    ElementTags tags;

    CoordinateSequence(std::uint64_t id, const Coordinates& coordinates, const ElementTags& tags) :
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
            return utymap::utils::GeoUtils::isPointInPolygon(c, coordinates.begin(), coordinates.end());
        });
    }

private:

    inline GeoCoordinate first() const { return coordinates[0]; }

    inline GeoCoordinate last() const { return coordinates[coordinates.size() - 1]; }

    inline void addToBegin(const Coords& other) { coordinates.insert(coordinates.begin(), other.begin(), other.end()); }

    inline void addToEnd(const Coords& other) { coordinates.insert(coordinates.end(), other.begin(), other.end()); }

    inline void reverse() { std::reverse(coordinates.begin(), coordinates.end()); }

    inline void mergeTags(const ElementTags& other) { tags.insert(tags.end(), other.begin(), other.end()); }
};


MultipolygonProcessor::MultipolygonProcessor(std::uint64_t id, RelationMembers& members, const Tags& tags, StringTable& stringTable,
    std::unordered_map<std::uint64_t, std::shared_ptr<utymap::entities::Area>>& areaMap,
    std::unordered_map<std::uint64_t, std::shared_ptr<utymap::entities::Way>>& wayMap) :
    id_(id), members_(members), tags_(tags), stringTable_(stringTable), areaMap_(areaMap), wayMap_(wayMap)
{
}

// see http://wiki.openstreetmap.org/wiki/Relation:multipolygon/Algorithm
Relation MultipolygonProcessor::process()
{
    Relation relation;
    relation.id = id_;

    bool allClosed = true;

    Ints outerIndecies;
    Ints innerIndecies;
    CoordinateSequences sequences;
    for (const auto& member : members_)
    {
        if (member.type != "way") continue;

        Coordinates coordinates;
        ElementTags tags;
        auto wayPair = wayMap_.find(member.refId);
        if (wayPair != wayMap_.end()) {
            coordinates = wayPair->second->coordinates;
            tags = wayPair->second->tags;
        }
        else {
            auto areaPair = areaMap_.find(member.refId);
            if (areaPair != areaMap_.end()) {
                coordinates = areaPair->second->coordinates;
                tags = areaPair->second->tags;
            }
            else {
                //  NOTE cannot fill relation: incomplete data
                return relation;
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
        std::shared_ptr<CoordinateSequence> sequence(new CoordinateSequence(id_, coordinates, tags));
        if (!sequence->isClosed()) allClosed = false;
        sequences.push_back(sequence);
    }

    if (outerIndecies.size() == 1 && allClosed)
        simpleCase(relation, sequences, outerIndecies, innerIndecies);
    else
        complexCase(relation, sequences);

    return std::move(relation);
}

std::vector<utymap::entities::Tag> MultipolygonProcessor::convertTags(const utymap::formats::Tags& tags) const
{
    std::vector<utymap::entities::Tag> convertedTags(tags.size());
    std::transform(tags.begin(), tags.end(), convertedTags.begin(), [&](const utymap::formats::Tag& tag) {
        return utymap::entities::Tag{
            stringTable_.getId(tag.key),
            stringTable_.getId(tag.value)
        };
    });
    return std::move(convertedTags);
}

std::vector<utymap::entities::Tag> MultipolygonProcessor::getTags(const CoordinateSequence& outer) const
{
    // TODO investigate case of empty tags
    auto tags = tags_.size() > 1 ? convertTags(tags_) : outer.tags;
    std::sort(tags.begin(), tags.end());
    return std::move(tags);
}

void MultipolygonProcessor::simpleCase(Relation& relation, const CoordinateSequences& sequences, const Ints& outerIndecies, const Ints& innerIndecies)
{
    // TODO set correct tags!
    auto outer = sequences[outerIndecies[0]];

    // TODO investigate case of empty tags
    relation.tags = getTags(*outer);
    if (relation.tags.empty()) return;

    // outer
    std::shared_ptr<Area> outerArea(new Area());
    outerArea->id = outer->id;
    outerArea->coordinates.insert(outerArea->coordinates.end(), outer->coordinates.begin(), outer->coordinates.end());
    relation.elements.push_back(outerArea);

    // inner
    for (int i : innerIndecies) {
        auto coords = sequences[i]->coordinates;
        std::shared_ptr<Area> innerArea(new Area());
        // TODO ensure hole orientation
        innerArea->coordinates.insert(innerArea->coordinates.end(), coords.rbegin(), coords.rend());
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
                isFound = true;
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
    }

    return std::move(closedRings);
}

void MultipolygonProcessor::fillRelation(Relation& relation, CoordinateSequences& rings)
{
    while (!rings.empty()) {
        // find an outer ring
        std::shared_ptr<CoordinateSequence> outer = nullptr;
        for (auto candidate = rings.begin(); candidate != rings.end(); ++candidate) {
            bool containedInOtherRings = false;
            for (auto other = rings.begin(); other != rings.end(); ++other) {
                if (other != candidate && (*other)->containsRing((*candidate)->coordinates)) {
                    containedInOtherRings = true;
                    break;
                }
            }
            if (containedInOtherRings) continue;
            outer = *candidate;
            rings.erase(candidate);
            break;
        }

        // find inner rings of that ring
        CoordinateSequences inners;
        for (auto ring = rings.begin(); ring != rings.end();) {
            if (outer->containsRing((*ring)->coordinates)) {
                bool containedInOthers = false;
                for (auto other = rings.begin(); other != rings.end(); ++other) {
                    if (other != ring && (*other)->containsRing((*ring)->coordinates)) {
                        containedInOthers = true;
                        break;
                    }
                }
                if (!containedInOthers) {
                    inners.push_back(*ring);
                    ring = rings.erase(ring);
                    continue;
                }
            }
            ++ring;
        }

        relation.tags = getTags(*outer);
        if (relation.tags.empty()) return;

        // outer
        std::shared_ptr<Area> outerArea(new Area());
        outerArea->id = outer->id;
        outerArea->coordinates.insert(outerArea->coordinates.end(), outer->coordinates.begin(), outer->coordinates.end());
        relation.elements.push_back(outerArea);

        // inner: create a new area and remove the used rings
        for (const auto& innerRing : inners) {
            std::shared_ptr<Area> innerArea(new Area());
            innerArea->coordinates.insert(innerArea->coordinates.end(), innerRing->coordinates.rbegin(), innerRing->coordinates.rend());
            relation.elements.push_back(innerArea);
        }
    }
}