#ifndef FORMATS_OSM_RELATIONPROCESSOR_HPP_DEFINED
#define FORMATS_OSM_RELATIONPROCESSOR_HPP_DEFINED

#include "formats/FormatTypes.hpp"
#include "formats/osm/OsmDataContext.hpp"
#include "utils/ElementUtils.hpp"

#include <cstdint>
#include <functional>
#include <memory>

namespace utymap { namespace formats {

class RelationProcessor
{
public:

    RelationProcessor(utymap::entities::Relation& relation,
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
    }

    void visit(OsmDataContext::WayMapType::const_iterator way)
    {
        relation_.elements.push_back(way->second);
    }

    void visit(OsmDataContext::AreaMapType::const_iterator area)
    {
        relation_.elements.push_back(area->second);
    }

    void visit(OsmDataContext::RelationMapType::const_iterator rel, const std::string& role)
    {
        resolve_(*rel->second);
        relation_.elements.push_back(rel->second);
    }

private:

    utymap::entities::Relation& relation_;
    const utymap::formats::RelationMembers& members_;
    utymap::formats::OsmDataContext& context_;
    std::function<void(utymap::entities::Relation&)> resolve_;
};

}}

#endif // FORMATS_OSM_RELATIONPROCESSOR_HPP_DEFINED
