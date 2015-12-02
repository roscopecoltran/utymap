#ifndef TESTS_FORMATS_OSMDATAVISITOR_HPP_DEFINED
#define TESTS_FORMATS_OSMDATAVISITOR_HPP_DEFINED

#include "BoundingBox.hpp"
#include "GeoCoordinate.hpp"
#include "formats/FormatTypes.hpp"

#include <cstdint>

using namespace utymap::formats;

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

    void visitNode(uint64_t id, utymap::GeoCoordinate& coordinate, TagCollection& tags)
    {
        nodes++;
    }

    void visitWay(uint64_t id, std::vector<uint64_t>& nodeIds, TagCollection& tags)
    {
        ways++;
    }

    void visitRelation(uint64_t id, RelationMembers& members, TagCollection& tags)
    {
        relations++;
    }
};

#endif // TESTS_FORMATS_OSMDATAVISITOR_HPP_DEFINED
