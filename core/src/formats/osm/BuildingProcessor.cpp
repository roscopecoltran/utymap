#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "formats/osm/BuildingProcessor.hpp"
#include "utils/ElementUtils.hpp"

using namespace utymap;
using namespace utymap::entities;
using namespace utymap::formats;
using namespace utymap::index;

BuildingProcessor::BuildingProcessor(
        std::uint64_t id,
        RelationMembers& members,
        const Tags& tags,
        StringTable& stringTable,
        OsmDataContext context)
    : id_(id), members_(members), tags_(tags),
      stringTable_(stringTable), context_(context)
{
}

void BuildingProcessor::process()
{
    std::shared_ptr<Relation> relation(new Relation());
    relation->id = id_;
    relation->tags = utymap::utils::convertTags(stringTable_, tags_);
    
    // TODO

    removeBuildingParts();
    context_.relationMap[id_] = relation;
}

void BuildingProcessor::removeBuildingParts()
{
    // NOTE Remove member references from other collections as
    // they will be processed as part of single relation.
    for (const auto& member : members_) {
        if (member.type == "area") {
            context_.areaMap.erase(member.refId);
        } else if (member.type == "relation") {
            context_.relationMap.erase(member.refId);
        }
    }
}