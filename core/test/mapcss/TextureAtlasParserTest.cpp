#include "mapcss/TextureAtlasParser.hpp"

#include <boost/test/unit_test.hpp>

using namespace utymap::mapcss;
using namespace utymap::meshing;

namespace {
    const double precision = 1E-9;

    const std::string SimpleContent = "300,300\nsimple,0,0,300,300";
}

BOOST_AUTO_TEST_SUITE(MapCss_TextureAtlasParser)

BOOST_AUTO_TEST_CASE(GivenSimpleContent_WhenParse_ThenAtlasReturned)
{
    //auto coord = TextureAtlasParser::parse(SimpleContent)
    //                    .get("simple")
    //                    .random(0)
    //                    .map(Vector2(0.5, 0.5));

    //BOOST_CHECK_CLOSE(coord.x, 0.5, precision);
    //BOOST_CHECK_CLOSE(coord.y, 0.5, precision);
}

BOOST_AUTO_TEST_SUITE_END()