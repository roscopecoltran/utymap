#ifndef TEST_MAPCSSUTILS_HPP_DEFINED
#define TEST_MAPCSSUTILS_HPP_DEFINED

#include "index/StringTable.hpp"
#include "index/StyleFilter.hpp"
#include "mapcss/MapCssParser.hpp"
#include "mapcss/StyleSheet.hpp"

#include <boost/test/unit_test.hpp>

#include <fstream>
#include <string>

class MapCssUtils
{
public:

    static utymap::index::StyleFilter createStyleFilterFromFile(utymap::index::StringTable& stringTable,
                                                                const std::string& filePath)
    {
        std::ifstream styleFile(filePath);
        utymap::mapcss::Parser parser;
        utymap::mapcss::StyleSheet stylesheet = parser.parse(styleFile);
        BOOST_TEST_CHECK(parser.getError().empty());
        return utymap::index::StyleFilter(stylesheet, stringTable);
    }

    static utymap::index::StyleFilter createStyleFilterFromString(utymap::index::StringTable& stringTable,
                                                                  const std::string& str)
    {
        utymap::mapcss::Parser parser;
        utymap::mapcss::StyleSheet stylesheet = parser.parse(str);
        BOOST_TEST_CHECK(parser.getError().empty());
        return utymap::index::StyleFilter(stylesheet, stringTable);
    }
};


#endif // TEST_MAPCSSUTILS_HPP_DEFINED
