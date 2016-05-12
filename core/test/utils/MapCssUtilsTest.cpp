#include "GeoCoordinate.hpp"
#include "entities/Way.hpp"
#include "mapcss/StyleProvider.hpp"
#include "utils/MapCssUtils.hpp"

#include <boost/test/unit_test.hpp>

#include "test_utils/DependencyProvider.hpp"
#include "test_utils/ElementUtils.hpp"

using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::mapcss;
using namespace utymap::utils;

namespace {
    const std::string stylesheet = "way|z15[meters] { width: 10m; }"
                                   "way|z15[percent] { width: 10%; }"
                                   "way|z15[water] { width: -1m; }";
}

struct Utils_MapCssUtilsFixture
{
    DependencyProvider dependencyProvider;
};

BOOST_FIXTURE_TEST_SUITE(Utils_MapCssUtils, Utils_MapCssUtilsFixture)

BOOST_AUTO_TEST_CASE(GivenValueInMeters_WhenGetDimension_ThenReturnPositiveValue)
{
    int lod = 15;
    Way way = ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(),
    { std::make_pair("meters", "") },
    { { 52.52975 , 13.38810 } });

    double width = getDimension("width", 
                                *dependencyProvider.getStringTable(),
                                dependencyProvider.getStyleProvider(stylesheet)->forElement(way, lod), 
                                lod, 
                                way.coordinates[0]);

    BOOST_CHECK(width > 0);
}

BOOST_AUTO_TEST_CASE(GivenValueInPercents_WhenGetDimension_ThenReturnPositiveValue)
{
    int lod = 15;
    Way way = ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(),
    { std::make_pair("percent", "") },
    { { 52.52975, 13.38810 } });

    double width = getDimension("width",
                                *dependencyProvider.getStringTable(),
                                dependencyProvider.getStyleProvider(stylesheet)->forElement(way, lod),
                                lod,
                                way.coordinates[0]);

    BOOST_CHECK(width > 0);
}

BOOST_AUTO_TEST_CASE(GivenNegativeValueInMeters_WhenGetDimension_ThenReturnExpectedValue)
{
    int lod = 15;
    Way way = ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(),
    { std::make_pair("water", "") },
    { { 52.52975, 13.38810 } });

    double width = getDimension("width",
                                *dependencyProvider.getStringTable(),
                                dependencyProvider.getStyleProvider(stylesheet)->forElement(way, lod),
                                1);

    BOOST_CHECK_EQUAL(width, -1);
}


BOOST_AUTO_TEST_SUITE_END()
