#ifndef INDEX_OSMDATAVISITOR_HPP_DEFINED
#define INDEX_OSMDATAVISITOR_HPP_DEFINED

#include "BoundingBox.hpp"
#include "GeoCoordinate.hpp"
#include "entities/Element.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "formats/FormatTypes.hpp"
#include "index/ElementStore.hpp"
#include "index/LodRange.hpp"
#include "index/StringTable.hpp"
#include "mapcss/StyleProvider.hpp"
#include "utils/ElementUtils.hpp"

#include <memory>
#include <unordered_map>

namespace utymap { namespace index {

struct OsmDataVisitor
{
    int bounds;
    int nodes;
    int areas;
    int ways;
    int relations;

    OsmDataVisitor(ElementStore& elementStore, const utymap::mapcss::StyleProvider& styleProvider, 
                   StringTable& stringTable, const LodRange& lodRange) :
        elementStore_(elementStore), styleProvider_(styleProvider), stringTable_(stringTable), 
        lodRange_(lodRange), bounds(0), nodes(0), ways(0), areas(0), relations(0)
    {
    }

    void visitBounds(utymap::BoundingBox bbox)
    {
        bounds++;
    }

    void visitNode(std::uint64_t id, utymap::GeoCoordinate& coordinate, utymap::formats::Tags& tags)
    {
        std::shared_ptr<utymap::entities::Node> node(new utymap::entities::Node());
        node->id = id;
        node->coordinate = coordinate;
        utymap::utils::setTags(stringTable_, *node, tags);
        nodeMap_[id] = node;

        if(elementStore_.store(*node, lodRange_, styleProvider_)) 
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
private:

    ElementStore& elementStore_;
    const utymap::mapcss::StyleProvider& styleProvider_;
    StringTable& stringTable_;
    const LodRange& lodRange_;

    std::unordered_map<std::uint64_t, std::shared_ptr<utymap::entities::Node>> nodeMap_;
    std::unordered_map<std::uint64_t, std::shared_ptr<utymap::entities::Way>> wayMap_;
    std::unordered_map<std::uint64_t, std::shared_ptr<utymap::entities::Area>> areaMap_;
};

}}

#endif // INDEX_OSMDATAVISITOR_HPP_DEFINED
