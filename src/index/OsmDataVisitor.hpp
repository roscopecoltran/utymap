#ifndef INDEX_OSMDATAVISITOR_HPP_DEFINED
#define INDEX_OSMDATAVISITOR_HPP_DEFINED

#include "BoundingBox.hpp"
#include "GeoCoordinate.hpp"
#include "entities/Element.hpp"
#include "formats/FormatTypes.hpp"
#include "index/ElementStore.hpp"
#include "index/LodRange.hpp"
#include "index/StringTable.hpp"
#include "mapcss/StyleProvider.hpp"
#include "utils/ElementUtils.hpp"

#include <string>
#include <memory>
#include <vector>
#include <unordered_set>
#include <unordered_map>

namespace utymap { namespace index {

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

    OsmDataVisitor(ElementStore& elementStore, 
                   const utymap::mapcss::StyleProvider& styleProvider,
                   StringTable& stringTable, 
                   const LodRange& lodRange);

    void visitBounds(utymap::BoundingBox bbox);

    void visitNode(std::uint64_t id, utymap::GeoCoordinate& coordinate, utymap::formats::Tags& tags);

    void visitWay(std::uint64_t id, std::vector<std::uint64_t>& nodeIds, utymap::formats::Tags& tags);

    void visitRelation(std::uint64_t id, utymap::formats::RelationMembers& members, utymap::formats::Tags& tags);

private:

    bool isArea(const utymap::formats::Tags& tags);
    bool hasTag(const std::string& key, const std::string& value, const utymap::formats::Tags& tags);

    ElementStore& elementStore_;
    const utymap::mapcss::StyleProvider& styleProvider_;
    StringTable& stringTable_;
    const LodRange& lodRange_;

    std::unordered_map<std::uint64_t, utymap::GeoCoordinate> nodeMap_;
    std::unordered_map<std::uint64_t, std::shared_ptr<utymap::entities::Way>> wayMap_;
    std::unordered_map<std::uint64_t, std::shared_ptr<utymap::entities::Area>> areaMap_;
};

}}

#endif // INDEX_OSMDATAVISITOR_HPP_DEFINED
