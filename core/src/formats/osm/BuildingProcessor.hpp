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
                      utymap::formats::OsmDataContext& context)
        : id_(id), members_(members), tags_(tags),
        stringTable_(stringTable), context_(context), relation_()
    {
    }

    void process()
    {
        relation_ = std::shared_ptr<utymap::entities::Relation>(new utymap::entities::Relation());
        relation_->id = id_;
        relation_->tags = utymap::utils::convertTags(stringTable_, tags_);

        utymap::utils::visitRelationMembers(context_, members_, *this);

        context_.relationMap[id_] = relation_;
    }

    void visit(OsmDataContext::NodeMapType::const_iterator node)
    {
        relation_->elements.push_back(node->second);
        context_.nodeMap.erase(node->first);
    }

    void visit(OsmDataContext::WayMapType::const_iterator way)
    {
        relation_->elements.push_back(way->second);
        context_.wayMap.erase(way->first);
    }

    void visit(OsmDataContext::AreaMapType::const_iterator area)
    {
        relation_->elements.push_back(area->second);
        context_.areaMap.erase(area->first);
    }

    void visit(OsmDataContext::RelationMapType::const_iterator rel, const std::string& role)
    {
        if (role != "outline")
            relation_->elements.push_back(rel->second);

        context_.relationMap.erase(rel->first);
    }

private:

    std::uint64_t id_;
    utymap::formats::RelationMembers& members_;
    const utymap::formats::Tags& tags_;
    utymap::index::StringTable& stringTable_;
    utymap::formats::OsmDataContext& context_;
    std::shared_ptr<utymap::entities::Relation> relation_;
};

}}

#endif // FORMATS_OSM_BUILDINGPROCESSOR_HPP_DEFINED