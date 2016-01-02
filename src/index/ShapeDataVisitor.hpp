#ifndef INDEX_SHAPEDATAVISITOR_HPP_DEFINED
#define INDEX_SHAPEDATAVISITOR_HPP_DEFINED

#include "BoundingBox.hpp"
#include "GeoCoordinate.hpp"
#include "formats/FormatTypes.hpp"
#include "formats/shape/ShapeParser.hpp"
#include "index/ElementStore.hpp"
#include "index/StringTable.hpp"
#include "index/StyleFilter.hpp"

namespace utymap { namespace index {

struct ShapeDataVisitor
{
    int bounds;
    int nodes;
    int ways;
    int relations;

    ShapeDataVisitor(ElementStore& store, StringTable& stringTable, const StyleFilter& styleFilter) :
        store_(store),
        stringTable_(stringTable),
        styleFilter_(styleFilter),
        bounds(0),
        nodes(0),
        ways(0),
        relations(0)
    {
    }

    void visitNode(utymap::GeoCoordinate& coordinate, utymap::formats::Tags& tags)
    {
        nodes++;
    }

    void visitWay(utymap::formats::Coordinates& coordinates, utymap::formats::Tags& tags, bool isRing)
    {
        ways++;
    }

    void visitRelation(utymap::formats::PolygonMembers& members, utymap::formats::Tags& tags)
    {
        relations++;
    }
private:
    ElementStore& store_;
    StringTable& stringTable_;
    const StyleFilter& styleFilter_;
};

}}

#endif // INDEX_SHAPEDATAVISITOR_HPP_DEFINED
