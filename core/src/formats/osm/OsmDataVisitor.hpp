#ifndef INDEX_OSMDATAVISITOR_HPP_DEFINED
#define INDEX_OSMDATAVISITOR_HPP_DEFINED

#include "BoundingBox.hpp"
#include "GeoCoordinate.hpp"
#include "entities/Element.hpp"
#include "formats/FormatTypes.hpp"
#include "index/StringTable.hpp"
#include "utils/ElementUtils.hpp"

#include <functional>
#include <string>
#include <memory>
#include <vector>
#include <unordered_set>
#include <unordered_map>

namespace utymap { namespace formats {

class OsmDataVisitor
{
    static std::unordered_set<std::string> AreaKeys;
    static std::unordered_set<std::string> FalseKeys;

public:

    struct Statistics
    {
        int bounds, nodes, areas, ways, relations;
        int skipNodes, skipWays, skipRelations;

        Statistics() : bounds(0), nodes(0), ways(0), areas(0), relations(0),
            skipNodes(0), skipWays(0), skipRelations(0)
        {
        }
    };

    Statistics statistics;

    OsmDataVisitor(utymap::index::StringTable& stringTable, std::function<bool(utymap::entities::Element&)> functor);

    void visitBounds(utymap::BoundingBox bbox);

    void visitNode(std::uint64_t id, utymap::GeoCoordinate& coordinate, utymap::formats::Tags& tags);

    void visitWay(std::uint64_t id, std::vector<std::uint64_t>& nodeIds, utymap::formats::Tags& tags);

    void visitRelation(std::uint64_t id, utymap::formats::RelationMembers& members, utymap::formats::Tags& tags);

private:

    bool isArea(const utymap::formats::Tags& tags) const;
    bool hasTag(const std::string& key, const std::string& value, const utymap::formats::Tags& tags) const;

    std::function<bool(utymap::entities::Element&)> functor_;
    utymap::index::StringTable& stringTable_;
    

    std::unordered_map<std::uint64_t, utymap::GeoCoordinate> nodeMap_;
    std::unordered_map<std::uint64_t, std::shared_ptr<utymap::entities::Way>> wayMap_;
    std::unordered_map<std::uint64_t, std::shared_ptr<utymap::entities::Area>> areaMap_;
};

}}

#endif // INDEX_OSMDATAVISITOR_HPP_DEFINED
