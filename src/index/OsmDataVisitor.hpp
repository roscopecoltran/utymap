#ifndef INDEX_OSMDATAVISITOR_HPP_DEFINED
#define INDEX_OSMDATAVISITOR_HPP_DEFINED

#include "BoundingBox.hpp"
#include "GeoCoordinate.hpp"
#include "formats/FormatTypes.hpp"

namespace utymap { namespace index {

struct OsmDataVisitor
{
    int bounds;
    int nodes;
    int ways;
    int relations;

    OsmDataVisitor() : bounds(0), nodes(0), ways(0), relations(0) {}

    void visitBounds(utymap::BoundingBox bbox)
    {
        bounds++;
    }

    void visitNode(std::uint64_t id, utymap::GeoCoordinate& coordinate, utymap::formats::Tags& tags)
    {
        nodes++;
    }

    void visitWay(std::uint64_t id, std::vector<std::uint64_t>& nodeIds, utymap::formats::Tags& tags)
    {
        ways++;
    }

    void visitRelation(std::uint64_t id, utymap::formats::RelationMembers& members, utymap::formats::Tags& tags)
    {
        relations++;
    }
};

}}

#endif // INDEX_OSMDATAVISITOR_HPP_DEFINED
