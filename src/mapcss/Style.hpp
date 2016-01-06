#ifndef INDEX_RULE_HPP_INCLUDED
#define INDEX_RULE_HPP_INCLUDED

#include "Exceptions.hpp"
#include "utils/CompatibilityUtils.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace utymap { namespace mapcss {

// Represents style for element.
struct Style
{
public:

    Style() : declarations_(), isApplicable(false)
    {
    }

    bool isApplicable;

    inline bool has(uint32_t key)
    {
        return declarations_.find(key) != declarations_.end();
    }

    inline void put(uint32_t key, uint32_t value)
    {
        declarations_[key] = value;
    }

    inline uint32_t get(uint32_t key)
    {
        auto it = declarations_.find(key);
        if (it == declarations_.end())
            throw MapCssException(std::string("Cannot find declaration with the key:") + std::to_string(key));

        return it->second;
    }

private:
    std::unordered_map<uint32_t, uint32_t> declarations_;
};

}}
#endif  // INDEX_RULE_HPP_INCLUDED
