#ifndef FORMATS_OSM_RELATIONPROCESSOR_HPP_DEFINED
#define FORMATS_OSM_RELATIONPROCESSOR_HPP_DEFINED

#include "formats/FormatTypes.hpp"
#include "formats/osm/OsmDataContext.hpp"
#include "utils/ElementUtils.hpp"

#include <cstdint>
#include <memory>

namespace utymap { namespace formats {

class RelationProcessor
{
public:

    RelationProcessor(std::uint64_t id,
                      utymap::formats::RelationMembers& members,
                      const utymap::formats::Tags& tags,
                      utymap::index::StringTable& stringTable,
                      utymap::formats::OsmDataContext context)
        : id_(id), members_(members), tags_(tags),
        stringTable_(stringTable), context_(context), relation_()
    {
    }

    std::shared_ptr<utymap::entities::Relation> process()
    {
        relation_ = std::shared_ptr<utymap::entities::Relation>(new utymap::entities::Relation());
        relation_->id = id_;
        relation_->tags = utymap::utils::convertTags(stringTable_, tags_);
        relation_->elements.reserve(members_.size());

        utymap::utils::visitRelationMembers(context_, members_, *this);

        return relation_;
    }

    void visit(OsmDataContext::NodeMapType::const_iterator node) 
    { 
        relation_->elements.push_back(node->second); 
    }

    void visit(OsmDataContext::WayMapType::const_iterator way) 
    { 
        relation_->elements.push_back(way->second);
    }

    void visit(OsmDataContext::AreaMapType::const_iterator area) 
    { 
        relation_->elements.push_back(area->second);
    }

    void visit(OsmDataContext::RelationMapType::const_iterator rel) 
    { 
        relation_->elements.push_back(rel->second);
    }

private:

    std::uint64_t id_;
    utymap::formats::RelationMembers& members_;
    const utymap::formats::Tags& tags_;
    utymap::index::StringTable& stringTable_;
    utymap::formats::OsmDataContext context_;
    std::shared_ptr<utymap::entities::Relation> relation_;

};

}}

#endif // FORMATS_OSM_RELATIONPROCESSOR_HPP_DEFINED
