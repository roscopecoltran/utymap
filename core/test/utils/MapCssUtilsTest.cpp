#include "GeoCoordinate.hpp"
#include "entities/Way.hpp"
#include "mapcss/StyleProvider.hpp"
#include "utils/MapCssUtils.hpp"

#include <boost/test/unit_test.hpp>

#include "test_utils/MapCssUtils.hpp"
#include "test_utils/ElementUtils.hpp"

using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::mapcss;
using namespace utymap::utils;

struct Utils_MapCssUtilsFixture
{
    Utils_MapCssUtilsFixture() :
        stringTablePtr(new StringTable("")),
        styleProviderPtr(nullptr)
    {
        std::string stylesheet = "way|z15[meters] { width: 10m; }"
                                 "way|z15[percent] { width: 10%; }"
                                 "way|z15[water] { width: -1m; }";
        styleProviderPtr = MapCssUtils::createStyleProviderFromString(*stringTablePtr, stylesheet);
    }

    ~Utils_MapCssUtilsFixture()
    {
        delete stringTablePtr;
        
        std::remove("string.idx");
        std::remove("string.dat");
    }

    StringTable* stringTablePtr;
    std::shared_ptr<StyleProvider> styleProviderPtr;

};

BOOST_FIXTURE_TEST_SUITE(Utils_MapCssUtils, Utils_MapCssUtilsFixture)

BOOST_AUTO_TEST_CASE(GivenValueInMeters_WhenGetDimension_ThenReturnPositiveValue)
{
    int lod = 15;
    Way way = ElementUtils::createElement<Way>(*stringTablePtr, 
    { std::make_pair("meters", "") },
    { { 52.52975 , 13.38810 } }
    );

    double width = utymap::utils::getDimension("width", *stringTablePtr, 
        styleProviderPtr->forElement(way, lod), lod, way.coordinates[0]);

    BOOST_CHECK(width > 0);
}

BOOST_AUTO_TEST_CASE(GivenValueInPercents_WhenGetDimension_ThenReturnPositiveValue)
{
    int lod = 15;
    Way way = ElementUtils::createElement<Way>(*stringTablePtr,
    { std::make_pair("percent", "") },
    { { 52.52975, 13.38810 } }
    );

    double width = utymap::utils::getDimension("width", *stringTablePtr,
        styleProviderPtr->forElement(way, lod), 1, way.coordinates[0]);

    BOOST_CHECK(width > 0);
}

BOOST_AUTO_TEST_CASE(GivenNegativeValueInMeters_WhenGetDimension_ThenReturnExpectedValue)
{
    int lod = 15;
    Way way = ElementUtils::createElement<Way>(*stringTablePtr,
    { std::make_pair("water", "") },
    { { 52.52975, 13.38810 } }
    );

    double width = utymap::utils::getDimension("width", *stringTablePtr,
        styleProviderPtr->forElement(way, lod), 1);

    BOOST_CHECK_EQUAL(width, -1);
}


BOOST_AUTO_TEST_SUITE_END()
