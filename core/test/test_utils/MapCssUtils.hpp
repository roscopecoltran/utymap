#ifndef TEST_MAPCSSUTILS_HPP_DEFINED
#define TEST_MAPCSSUTILS_HPP_DEFINED

#include "index/StringTable.hpp"
#include "mapcss/MapCssParser.hpp"
#include "mapcss/StyleSheet.hpp"
#include "mapcss/StyleProvider.hpp"

#include <boost/test/unit_test.hpp>

#include <cstdint>
#include <fstream>
#include <string>

class MapCssUtils
{
public:

    static std::shared_ptr<utymap::mapcss::StyleProvider> createStyleProviderFromFile(utymap::index::StringTable& stringTable,
                                                                                      const std::string& filePath)
    {
        std::ifstream styleFile(filePath);
        utymap::mapcss::MapCssParser parser;
        utymap::mapcss::StyleSheet stylesheet = parser.parse(styleFile);
        return std::shared_ptr<utymap::mapcss::StyleProvider>(new utymap::mapcss::StyleProvider(stylesheet, stringTable));
    }

    static std::shared_ptr<utymap::mapcss::StyleProvider> createStyleProviderFromString(utymap::index::StringTable& stringTable,
                                                                                        const std::string& str)
    {
        utymap::mapcss::MapCssParser parser;
        utymap::mapcss::StyleSheet stylesheet = parser.parse(str);
        return std::shared_ptr<utymap::mapcss::StyleProvider>(new utymap::mapcss::StyleProvider(stylesheet, stringTable));
    }
};


#endif // TEST_MAPCSSUTILS_HPP_DEFINED
