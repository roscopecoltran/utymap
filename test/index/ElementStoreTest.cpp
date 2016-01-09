#include "GeoCoordinate.hpp"
#include "entities/Element.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "index/ElementStore.hpp"
#include "mapcss/StyleProvider.hpp"
#include "test_utils/ElementUtils.hpp"
#include "test_utils/MapCssUtils.hpp"

#include <boost/test/unit_test.hpp>

#include <functional>
#include <initializer_list>
#include <utility>

using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::mapcss;

typedef std::function<void(const Element&, const utymap::QuadKey&)> StoreCallback;

class TestElementStore : public ElementStore
{
public:

    int times;

    TestElementStore(const StyleProvider& styleProvider, StringTable& stringTable, StoreCallback function) :
        ElementStore(styleProvider, stringTable),
        function_(function),
        times(0)
    {
    }

protected:
    void storeImpl(const Element& element, const utymap::QuadKey& quadKey)
    {
        times++;
        function_(element, quadKey);
    }
private:
    StoreCallback function_;
};

struct Index_ElementStoreFixture
{
    Index_ElementStoreFixture() :
        stringTablePtr(new StringTable("")),
        styleProviderPtr(nullptr),
        elementStorePtr()
    {
        BOOST_TEST_MESSAGE("setup fixture");
    }

    ~Index_ElementStoreFixture()
    {
        BOOST_TEST_MESSAGE("teardown fixture");
        BOOST_TEST_CHECK(elementStorePtr->times > 0);
        delete elementStorePtr;
        delete styleProviderPtr;
        delete stringTablePtr;
        std::remove("string.idx");
        std::remove("string.dat");
    }

    void createElementStore(std::string stylesheet, const StoreCallback callback)
    {
        styleProviderPtr = MapCssUtils::createStyleProviderFromString(*stringTablePtr, stylesheet);
        elementStorePtr = new TestElementStore(*styleProviderPtr, *stringTablePtr, callback);
    }

    StringTable* stringTablePtr;
    StyleProvider* styleProviderPtr;
    TestElementStore* elementStorePtr;
};

bool checkQuadKey(const utymap::QuadKey& quadKey, int lod, int tileX, int tileY) 
{
    return quadKey.levelOfDetail == lod && 
           quadKey.tileX == tileX && 
           quadKey.tileY == tileY;
}

template<typename T>
void checkGeometry(const T& t, std::initializer_list<std::pair<double, double>> geometry)
{
    BOOST_CHECK_EQUAL(t.coordinates.size(), geometry.size());
    int i = 0;
    for (const auto& pair : geometry) {
        BOOST_CHECK_EQUAL(pair.first, t.coordinates[i].latitude);
        BOOST_CHECK_EQUAL(pair.second, t.coordinates[i].longitude);
        i++;
    }
}

BOOST_FIXTURE_TEST_SUITE(Index_ElementStore, Index_ElementStoreFixture)

BOOST_AUTO_TEST_CASE(GivenBigWayAtZoom1_WhenStore_GeometryIsClipped)
{
    Way way = ElementUtils::createElement<Way>(*stringTablePtr,
        { { "test", "Foo" } },
        { { 10, 10 }, { 10, -10 }});
    createElementStore("way|z1[test=Foo] { key:val; clip: true;}",
        [&](const Element& element, const utymap::QuadKey& quadKey) {
        if (checkQuadKey(quadKey, 1, 0, 0)) {
            checkGeometry<Way>(reinterpret_cast<const Way&>(element), { { 10, -10 }, { 10, 0 } });
        }
        else if (checkQuadKey(quadKey, 1, 1, 0)) {
            checkGeometry<Way>(reinterpret_cast<const Way&>(element), { { 10, 0 }, { 10, 10 } });
        }
        else {
            BOOST_TEST_FAIL("Unexpected quadKey!");
        }
    });

    elementStorePtr->store(way, LodRange(1,1));

    BOOST_CHECK_EQUAL(elementStorePtr->times, 2);
}

BOOST_AUTO_TEST_SUITE_END()
