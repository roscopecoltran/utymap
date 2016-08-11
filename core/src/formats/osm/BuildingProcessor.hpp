#ifndef FORMATS_OSM_BUILDINGPROCESSOR_HPP_DEFINED
#define FORMATS_OSM_BUILDINGPROCESSOR_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "entities/Element.hpp"
#include "formats/FormatTypes.hpp"
#include "formats/osm/OsmDataContext.hpp"
#include "utils/ElementUtils.hpp"

#include <functional>
#include <memory>
#include <unordered_map>

namespace utymap { namespace formats {

// Provides the way to process buildings parts as single mesh.
class BuildingProcessor
{
public:
    BuildingProcessor(utymap::entities::Relation& relation,
                      const utymap::formats::RelationMembers& members,
                      utymap::formats::OsmDataContext& context,
                      std::function<void(utymap::entities::Relation&)> resolve)
    : relation_(relation), members_(members), context_(context), resolve_(resolve)
    {
    }

    void process()
    {
        utymap::utils::visitRelationMembers(context_, members_, *this);
    }

    void visit(OsmDataContext::NodeMapType::const_iterator node)
    {
        relation_.elements.push_back(node->second);
        putMarkerTag(node->second);
    }

    void visit(OsmDataContext::WayMapType::const_iterator way)
    {
        relation_.elements.push_back(way->second);
        putMarkerTag(way->second);
    }

    void visit(OsmDataContext::AreaMapType::const_iterator area)
    {
        relation_.elements.push_back(area->second);
        putMarkerTag(area->second);
    }

    void visit(OsmDataContext::RelationMapType::const_iterator rel, const std::string& role)
    {
        resolve_(*rel->second);

        if (role != "outline")
            relation_.elements.push_back(rel->second);

        putMarkerTag(rel->second);
    }

private:

    // NOTE add specific tag to signalize that this element should not be processed separetely by building builder
    void putMarkerTag(std::shared_ptr<utymap::entities::Element> element) 
    {
        element->tags.push_back(utymap::entities::Tag(
            std::numeric_limits<std::uint32_t>::max(),
            std::numeric_limits<std::uint32_t>::max()));
    }

    utymap::entities::Relation& relation_;
    const utymap::formats::RelationMembers& members_;
    utymap::formats::OsmDataContext& context_;
    std::function<void(utymap::entities::Relation&)> resolve_;
};

}}

#endif // FORMATS_OSM_BUILDINGPROCESSOR_HPP_DEFINED
