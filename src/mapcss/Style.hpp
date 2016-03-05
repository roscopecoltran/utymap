#ifndef MAPCSS_STYLE_HPP_INCLUDED
#define MAPCSS_STYLE_HPP_INCLUDED

#include "Exceptions.hpp"
#include "utils/CompatibilityUtils.hpp"

#include <cstdint>
#include <string>
#include <memory>
#include <unordered_map>

namespace utymap { namespace mapcss {

// Represents style for element.
struct Style
{
    std::unordered_map<uint32_t, std::shared_ptr<std::string>> declarations;

    inline bool has(uint32_t key) const 
    {
        return declarations.find(key) != declarations.end();
    }

    inline bool has(uint32_t key, const std::string& value) const
    {
        auto it = declarations.find(key);
        return it != declarations.end() && *it->second == value;
    }

    inline void put(const uint32_t key, const std::shared_ptr<std::string>& value)
    {
        declarations[key] = value;
    }

    inline std::string& get(const uint32_t key) const
    {
        auto it = declarations.find(key);
        if (it == declarations.end())
            throw MapCssException(std::string("Cannot find declaration with the key:") + std::to_string(key));

        return *it->second;
    }
};

}}
#endif  // MAPCSS_STYLE_HPP_INCLUDED
