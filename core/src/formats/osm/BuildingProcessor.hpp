#ifndef FORMATS_OSM_BUILDINGPROCESSOR_HPP_DEFINED
#define FORMATS_OSM_BUILDINGPROCESSOR_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "entities/Element.hpp"
#include "formats/FormatTypes.hpp"
#include "formats/osm/OsmDataContext.hpp"

#include <memory>
#include <unordered_map>

namespace utymap { namespace formats {

// Provides the way to process buildings parts as single mesh.
class BuildingProcessor
{
public:
    BuildingProcessor(std::uint64_t id,
                      utymap::formats::RelationMembers& members,
                      const utymap::formats::Tags& tags,
                      utymap::index::StringTable& stringTable,
                      utymap::formats::OsmDataContext context);

    std::shared_ptr<utymap::entities::Relation> process();

private:

    void removeBuildingParts();

    std::uint64_t id_;
    utymap::formats::RelationMembers& members_;
    const utymap::formats::Tags& tags_;
    utymap::index::StringTable& stringTable_;
    utymap::formats::OsmDataContext context_;
};

}}

#endif // FORMATS_OSM_BUILDINGPROCESSOR_HPP_DEFINED