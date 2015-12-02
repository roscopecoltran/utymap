#ifndef TESTS_FORMATS_SHAPEDATAVISITOR_HPP_DEFINED
#define TESTS_FORMATS_SHAPEDATAVISITOR_HPP_DEFINED

#include "BoundingBox.hpp"
#include "GeoCoordinate.hpp"
#include "formats/FormatTypes.hpp"

#include <cstdint>

using namespace utymap::formats;

struct ShapeDataVisitor
{
    int bounds;
    int nodes;
    int ways;
    int relations;

    utymap::GeoCoordinate lastCoordinate;
    TagCollection lastTags;

    ShapeDataVisitor() : bounds(0), nodes(0), ways(0), relations(0),
        lastCoordinate(), lastTags() {}

    void visitBounds(utymap::BoundingBox bbox)
    {
        bounds++;
    }

    void visitNode(utymap::GeoCoordinate& coordinate, TagCollection& tags)
    {
        lastCoordinate = coordinate;
        lastTags = tags;
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

#endif // TESTS_FORMATS_SHAPEDATAVISITOR_HPP_DEFINED
