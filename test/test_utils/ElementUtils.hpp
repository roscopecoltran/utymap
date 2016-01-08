#ifndef TEST_ELEMENTUTILS_HPP_DEFINED
#define TEST_ELEMENTUTILS_HPP_DEFINED

#include "index/StringTable.hpp"
#include "mapcss/StyleProvider.hpp"

#include <boost/test/unit_test.hpp>

#include <cstdint>
#include <initializer_list>

class ElementUtils
{
public:

    template <typename T>
    static T createElement(utymap::index::StringTable& stringTable,
                           std::initializer_list<std::pair<const char*, const char*>> tags)
    {
        T t;
        for (auto pair : tags) {
            uint32_t key = stringTable.getId(pair.first);
            uint32_t value = stringTable.getId(pair.second);
            Tag tag(key, value);
            t.tags.push_back(tag);
        }
        return t;
    }
};


#endif // TEST_ELEMENTUTILS_HPP_DEFINED
