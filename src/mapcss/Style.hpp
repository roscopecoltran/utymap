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
    inline bool has(uint32_t key) const 
    {
        return declarations_.find(key) != declarations_.end();
    }

    inline void put(const uint32_t key, const std::shared_ptr<std::string>& value)
    {
        declarations_[key] = value;
    }

    inline std::string& get(const uint32_t key) const
    {
        auto it = declarations_.find(key);
        if (it == declarations_.end())
            throw MapCssException(std::string("Cannot find declaration with the key:") + std::to_string(key));

        return *it->second;
    }

private:
    std::unordered_map<uint32_t, std::shared_ptr<std::string>> declarations_;
};

}}
#endif  // MAPCSS_STYLE_HPP_INCLUDED
