#ifndef UTILS_MAPCSSUTILS_HPP_DEFINED
#define UTILS_MAPCSSUTILS_HPP_DEFINED

#include "index/StringTable.hpp"
#include "mapcss/Style.hpp"

#include <string>

namespace utymap { namespace utils {

    inline bool hasKey(const std::string& key,
                       utymap::index::StringTable& stringTable,
                       const utymap::mapcss::Style& style)
    {
        return style.has(stringTable.getId(key));
    }

    // Gets declaration value as string.
    inline std::string& getString(const std::string& key,
                                  utymap::index::StringTable& stringTable, 
                                  const utymap::mapcss::Style& style)
    {
        uint32_t keyId = stringTable.getId(key);
        return style.get(keyId);
    }

    // Gets declaration value as float.
    inline float getFloat(const std::string& key,
                          utymap::index::StringTable& stringTable,
                          const utymap::mapcss::Style& style)
    {
        uint32_t keyId = stringTable.getId(key);
        return std::stof(style.get(keyId));
    }

}}

#endif // UTILS_MAPCSSUTILS_HPP_DEFINED
