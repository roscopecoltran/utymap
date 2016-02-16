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

    inline std::string getString(const std::string& key,
                                 utymap::index::StringTable& stringTable,
                                 const utymap::mapcss::Style& style,
                                 const std::string defaultValue)
    {
        return utymap::utils::hasKey(key, stringTable, style) ? getString(key, stringTable, style) : defaultValue;
    }

    // Gets declaration value as float.
    inline float getDouble(const std::string& key,
                          utymap::index::StringTable& stringTable,
                          const utymap::mapcss::Style& style)
    {
        uint32_t keyId = stringTable.getId(key);
        return std::stod(style.get(keyId));
    }

    inline float getDouble(const std::string& key,
                           utymap::index::StringTable& stringTable,
                           const utymap::mapcss::Style& style,
                           double defaultValue)
    {
        return utymap::utils::hasKey(key, stringTable, style) ? getDouble(key, stringTable, style) : defaultValue;
    }
}}

#endif // UTILS_MAPCSSUTILS_HPP_DEFINED
