#ifndef UTILS_ELEMENTUTILS_HPP_DEFINED
#define UTILS_ELEMENTUTILS_HPP_DEFINED

#include "entities/Element.hpp"
#include "formats/FormatTypes.hpp"
#include "index/StringTable.hpp"

#include <algorithm>
#include <cstdint>

namespace utymap { namespace utils {

// Sets tags to element.
inline void setTags(utymap::index::StringTable& stringTable,
             utymap::entities::Element& element,
             const utymap::formats::Tags& tags)
{
    for (const auto& tag : tags) {
        std::uint32_t key = stringTable.getId(tag.key);
        std::uint32_t value = stringTable.getId(tag.value);
        element.tags.push_back(utymap::entities::Tag(key, value));
    }
    stringTable.flush();
    // NOTE: tags should be sorted to speed up mapcss styling
    std::sort(element.tags.begin(), element.tags.end());
}

}}

#endif // UTILS_ELEMENTUTILS_HPP_DEFINED
