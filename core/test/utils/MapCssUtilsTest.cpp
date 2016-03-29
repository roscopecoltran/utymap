#include "GeoCoordinate.hpp"
#include "entities/Way.hpp"
#include "index/StringTable.hpp"
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
        std::string stylesheet = "way|z15[width=meters] { width: 10m; }"
                                 "way|z15[width=grad] { width: 10; }";
        styleProviderPtr = MapCssUtils::createStyleProviderFromString(*stringTablePtr, stylesheet);
    }

    ~Utils_MapCssUtilsFixture()
    {
        delete stringTablePtr;
        delete styleProviderPtr;
        std::remove("string.idx");
        std::remove("string.dat");
    }

    StringTable* stringTablePtr;
    StyleProvider* styleProviderPtr;

};

BOOST_FIXTURE_TEST_SUITE(Utils_MapCssUtils, Utils_MapCssUtilsFixture)

BOOST_AUTO_TEST_CASE(GivenWidthInMeters_WhenGetWidth_ThenReturnGreaterThanZero)
{
    int lod = 15;
    Way way = ElementUtils::createElement<Way>(*stringTablePtr, 
    { std::make_pair("width", "meters") },
    { { 52.52975 , 13.38810 } }
    );

    double width = utymap::utils::getWidth("width", way.tags, *stringTablePtr, 
        styleProviderPtr->forElement(way, lod), way.coordinates[0], lod);

    BOOST_CHECK(width > 0);
}

BOOST_AUTO_TEST_CASE(GivenWidthInGrad_WhenGetWidth_ThenReturnGreaterThanZero)
{
    int lod = 15;
    Way way = ElementUtils::createElement<Way>(*stringTablePtr,
    { std::make_pair("width", "grad") },
    { { 52.52975, 13.38810 } }
    );

    double width = utymap::utils::getWidth("width", way.tags, *stringTablePtr,
        styleProviderPtr->forElement(way, lod), way.coordinates[0], lod);

    BOOST_CHECK(width > 0);
}

BOOST_AUTO_TEST_SUITE_END()
