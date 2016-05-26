#ifndef FORMATS_OSM_OSMDATACONTEXT_HPP_DEFINED
#define FORMATS_OSM_OSMDATACONTEXT_HPP_DEFINED

#include "entities/Element.hpp"

#include <memory>
#include <unordered_map>

namespace utymap { namespace formats {

// Specifies visitor context which stores elements.
struct OsmDataContext
{
    std::unordered_map<std::uint64_t, std::shared_ptr<utymap::entities::Node>> nodeMap;
    std::unordered_map<std::uint64_t, std::shared_ptr<utymap::entities::Way>> wayMap;
    std::unordered_map<std::uint64_t, std::shared_ptr<utymap::entities::Area>> areaMap;
    std::unordered_map<std::uint64_t, std::shared_ptr<utymap::entities::Relation>> relationMap;
};

}}

#endif // FORMATS_OSM_OSMDATACONTEXT_HPP_DEFINED