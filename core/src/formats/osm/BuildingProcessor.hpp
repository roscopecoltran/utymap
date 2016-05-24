#ifndef FORMATS_OSM_BUILDINGPROCESSOR_HPP_DEFINED
#define FORMATS_OSM_BUILDINGPROCESSOR_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "entities/Element.hpp"
#include "formats/FormatTypes.hpp"

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
                      std::unordered_map<std::uint64_t, std::shared_ptr<utymap::entities::Relation>>& relationMap,
                      std::unordered_map<std::uint64_t, std::shared_ptr<utymap::entities::Area>>& areaMap,
                      std::unordered_map<std::uint64_t, std::shared_ptr<utymap::entities::Way>>& wayMap);

    utymap::entities::Relation process();

private:
    std::uint64_t id_;
    utymap::formats::RelationMembers& members_;
    const utymap::formats::Tags& tags_;
    utymap::index::StringTable& stringTable_;
    std::unordered_map<std::uint64_t, std::shared_ptr<utymap::entities::Relation>>& relationMap_;
    std::unordered_map<std::uint64_t, std::shared_ptr<utymap::entities::Area>>& areaMap_;
    std::unordered_map<std::uint64_t, std::shared_ptr<utymap::entities::Way>>& wayMap_;
};

}}

#endif // FORMATS_OSM_BUILDINGPROCESSOR_HPP_DEFINED