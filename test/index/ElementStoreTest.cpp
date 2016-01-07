#include "index/ElementStore.hpp"
#include "mapcss/StyleProvider.hpp"

#include <boost/test/unit_test.hpp>

#include "test_utils/MapCssUtils.hpp"

using namespace utymap::index;
using namespace utymap::mapcss;

struct Index_ElementStoreFixture
{
    Index_ElementStoreFixture() :
        stringTablePtr(new StringTable("")),
        styleProviderPtr(nullptr)
    {
        BOOST_TEST_MESSAGE("setup fixture");
    }

    ~Index_ElementStoreFixture()
    {
        BOOST_TEST_MESSAGE("teardown fixture");
        delete stringTablePtr;
        delete styleProviderPtr;
        std::remove("string.idx");
        std::remove("string.dat");
    }

    StringTable* stringTablePtr;
    StyleProvider* styleProviderPtr;
};

BOOST_FIXTURE_TEST_SUITE(Index_ElementStore, Index_ElementStoreFixture)

BOOST_AUTO_TEST_CASE(GivenBigWay_WhenStore_GeometryIsClipped)
{
    styleProviderPtr = MapCssUtils::createStyleProviderFromString(*stringTablePtr, 
        "way|z1[test=Foo] { key:val; }");

    // TODO
}

BOOST_AUTO_TEST_SUITE_END()