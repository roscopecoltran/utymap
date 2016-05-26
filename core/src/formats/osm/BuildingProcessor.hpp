#ifndef FORMATS_OSM_BUILDINGPROCESSOR_HPP_DEFINED
#define FORMATS_OSM_BUILDINGPROCESSOR_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "entities/Element.hpp"
#include "formats/FormatTypes.hpp"
#include "formats/osm/OsmDataContext.hpp"
#include "utils/ElementUtils.hpp"

#include <memory>
#include <unordered_map>

namespace utymap { namespace formats {

// Provides the way to process buildings parts as single mesh.
class BuildingProcessor
{
public:
    BuildingProcessor(std::uint64_t id,
                      utymap::formats::RelationMembers& members,
                      const utymap::formats::Tags& tags,
                      utymap::index::StringTable& stringTable,
                      utymap::formats::OsmDataContext context) 
        : id_(id), members_(members), tags_(tags),
          stringTable_(stringTable), context_(context)
    {
    }

    void BuildingProcessor::process()
    {
        std::shared_ptr<utymap::entities::Relation> relation(new utymap::entities::Relation());
        relation->id = id_;
        relation->tags = utymap::utils::convertTags(stringTable_, tags_);

        // remove children references
        utymap::utils::visitRelationMembers(context_, members_, *this);

        context_.relationMap[id_] = relation;
    }

    void visit(OsmDataContext::NodeMapType::const_iterator node)
    {
        context_.nodeMap.erase(node->first);
    }

    void visit(OsmDataContext::WayMapType::const_iterator way)
    {
        context_.wayMap.erase(way->first);
    }

    void visit(OsmDataContext::AreaMapType::const_iterator area)
    {
        context_.areaMap.erase(area->first);
    }

    void visit(OsmDataContext::RelationMapType::const_iterator rel)
    {
        context_.relationMap.erase(rel->first);
    }

private:

    std::uint64_t id_;
    utymap::formats::RelationMembers& members_;
    const utymap::formats::Tags& tags_;
    utymap::index::StringTable& stringTable_;
    utymap::formats::OsmDataContext context_;
};

}}

#endif // FORMATS_OSM_BUILDINGPROCESSOR_HPP_DEFINED