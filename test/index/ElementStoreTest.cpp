#include "entities/Element.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "index/ElementStore.hpp"
#include "mapcss/StyleProvider.hpp"

#include <boost/test/unit_test.hpp>

#include "test_utils/MapCssUtils.hpp"

using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::mapcss;

class TestElementStore : public ElementStore
{
public:
    TestElementStore(const StyleProvider& styleProvider, StringTable& stringTable) :
        ElementStore(styleProvider, stringTable),
        lastElement(),
        lastQuadKey()
    {
    }

    Element lastElement;
    utymap::QuadKey lastQuadKey;

protected:
    void storeImpl(const Element& element, const utymap::QuadKey& quadKey)
    {
        lastElement = element;
        lastQuadKey = quadKey;
    }
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
        delete elementStorePtr;
        delete styleProviderPtr;
        delete stringTablePtr;
        std::remove("string.idx");
        std::remove("string.dat");
    }

    void createElementStore(std::string stylesheet)
    {
        styleProviderPtr = MapCssUtils::createStyleProviderFromString(*stringTablePtr, stylesheet);
        elementStorePtr = new TestElementStore(*styleProviderPtr, *stringTablePtr);
    }

    StringTable* stringTablePtr;
    StyleProvider* styleProviderPtr;
    TestElementStore* elementStorePtr;
};

BOOST_FIXTURE_TEST_SUITE(Index_ElementStore, Index_ElementStoreFixture)

BOOST_AUTO_TEST_CASE(GivenBigWay_WhenStore_GeometryIsClipped)
{
    Way way;
    createElementStore("way|z1[test=Foo] { key:val; }");

    elementStorePtr->store(way);
    // TODO
}

BOOST_AUTO_TEST_SUITE_END()
