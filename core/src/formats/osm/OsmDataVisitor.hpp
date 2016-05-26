#ifndef FORMATS_OSM_OSMDATAVISITOR_HPP_DEFINED
#define FORMATS_OSM_OSMDATAVISITOR_HPP_DEFINED

#include "BoundingBox.hpp"
#include "GeoCoordinate.hpp"
#include "entities/Element.hpp"
#include "formats/FormatTypes.hpp"
#include "formats/osm/OsmDataContext.hpp"
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
public:

    OsmDataVisitor(utymap::index::StringTable& stringTable,
                   std::function<bool(utymap::entities::Element&)> add);

    void visitBounds(utymap::BoundingBox bbox);

    void visitNode(std::uint64_t id, utymap::GeoCoordinate& coordinate, utymap::formats::Tags& tags);

    void visitWay(std::uint64_t id, std::vector<std::uint64_t>& nodeIds, utymap::formats::Tags& tags);

    void visitRelation(std::uint64_t id, utymap::formats::RelationMembers& members, utymap::formats::Tags& tags);

    void complete();

private:

    bool isArea(const utymap::formats::Tags& tags) const;
    bool hasTag(const std::string& key, const std::string& value, const utymap::formats::Tags& tags) const;
    
    utymap::index::StringTable& stringTable_;
    std::function<bool(utymap::entities::Element&)> add_;
    utymap::formats::OsmDataContext context_;
};

}}

#endif // FORMATS_OSM_OSMDATAVISITOR_HPP_DEFINED
