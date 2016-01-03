#ifndef INDEX_SHAPEDATAVISITOR_HPP_DEFINED
#define INDEX_SHAPEDATAVISITOR_HPP_DEFINED

#include "BoundingBox.hpp"
#include "GeoCoordinate.hpp"
#include "formats/FormatTypes.hpp"
#include "formats/shape/ShapeParser.hpp"
#include "index/ElementStore.hpp"
#include "index/StringTable.hpp"
#include "index/StyleFilter.hpp"
#include "index/GeoUtils.hpp"

#include <cstdint>

namespace utymap { namespace index {

struct ShapeDataVisitor
{
    int bounds;
    int nodes;
    int ways;
    int relations;

    ShapeDataVisitor(ElementStore& elementStore) :
        elementStore_(elementStore),
        bounds(0),
        nodes(0),
        ways(0),
        relations(0)
    {
    }

    void visitNode(utymap::GeoCoordinate& coordinate, utymap::formats::Tags& tags)
    {
        ElementStore::GeoPolygon polygon;
        polygon.reserve(1);
        polygon.push_back(std::vector<GeoCoordinate> {coordinate});
        elementStore_.store(polygon, tags, ElementStore::Node);

        nodes++;
    }

    void visitWay(utymap::formats::Coordinates& coordinates, utymap::formats::Tags& tags, bool isRing)
    {
        ElementStore::GeoPolygon polygon;
        polygon.reserve(1);
        polygon.push_back(coordinates);

        elementStore_.store(polygon, tags, isRing ? ElementStore::Area : ElementStore::Way);
        ways++;
    }

    void visitRelation(utymap::formats::PolygonMembers& members, utymap::formats::Tags& tags)
    {
        // TODO
        relations++;
    }


private:
    ElementStore& elementStore_;
};

}}

#endif // INDEX_SHAPEDATAVISITOR_HPP_DEFINED
