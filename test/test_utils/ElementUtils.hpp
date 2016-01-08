#ifndef TEST_ELEMENTUTILS_HPP_DEFINED
#define TEST_ELEMENTUTILS_HPP_DEFINED

#include "GeoCoordinate.hpp"
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
        for (const auto& pair : tags) {
            uint32_t key = stringTable.getId(pair.first);
            uint32_t value = stringTable.getId(pair.second);
            Tag tag(key, value);
            t.tags.push_back(tag);
        }
        return std::move(t);
    }

    template <typename T>
    static T createElement(utymap::index::StringTable& stringTable,
                           std::initializer_list<std::pair<const char*, const char*>> tags,
                           std::initializer_list<std::pair<double, double>> geometry)
    {
        T t = createElement<T>(stringTable, tags);
        t.coordinates.reserve(geometry.size());;
        for (const auto& pair : geometry) {
            t.coordinates.push_back(utymap::GeoCoordinate(pair.first, pair.second));
        }
        return std::move(t);
    }
};


#endif // TEST_ELEMENTUTILS_HPP_DEFINED
