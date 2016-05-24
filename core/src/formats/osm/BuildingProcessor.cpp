#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "formats/osm/BuildingProcessor.hpp"

using namespace utymap;
using namespace utymap::entities;
using namespace utymap::formats;
using namespace utymap::index;

BuildingProcessor::BuildingProcessor(
        std::uint64_t id,
        RelationMembers& members,
        const Tags& tags,
        StringTable& stringTable,
        std::unordered_map<std::uint64_t, std::shared_ptr<Relation>>& relationMap,
        std::unordered_map<std::uint64_t, std::shared_ptr<Area>>& areaMap,
        std::unordered_map<std::uint64_t, std::shared_ptr<Way>>& wayMap)
    : id_(id), members_(members), tags_(tags), stringTable_(stringTable),
      relationMap_(relationMap), areaMap_(areaMap), wayMap_(wayMap)
{
}

utymap::entities::Relation BuildingProcessor::process()
{

}