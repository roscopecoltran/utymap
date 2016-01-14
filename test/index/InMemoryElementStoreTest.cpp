#include "QuadKey.hpp"
#include "entities/Element.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "index/InMemoryElementStore.hpp"
#include "index/LodRange.hpp"
#include "mapcss/StyleProvider.hpp"

#include <boost/test/unit_test.hpp>

#include "test_utils/ElementUtils.hpp"
#include "test_utils/MapCssUtils.hpp"

using namespace utymap;
using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::mapcss;

struct Index_InMemoryElementStoreFixture
{
    Index_InMemoryElementStoreFixture() :
        stringTablePtr(new StringTable("")),
        styleProviderPtr(nullptr),
        elementStorePtr()
    {
        BOOST_TEST_MESSAGE("setup fixture");
        LodRange range(1, 2);
        std::string stylesheet = "area|z1[any],way|z1[any],node|z1[any] { clip: true; }";
        styleProviderPtr = MapCssUtils::createStyleProviderFromString(*stringTablePtr, stylesheet);
        
        elementStorePtr = new InMemoryElementStore(*stringTablePtr);
        elementStorePtr->store(ElementUtils::createElement<Way>(*stringTablePtr,
            { { "any", "true" } }, { { 5, -5 }, { 5, -10 } }), range, *styleProviderPtr);
        elementStorePtr->store(ElementUtils::createElement<Area>(*stringTablePtr,
            { { "any", "true" } }, { { 5, -5 }, { 5, -10 }, {10, -10} }), range, *styleProviderPtr);
        Node node = ElementUtils::createElement<Node>(*stringTablePtr, { { "any", "true" } });
        node.coordinate = {5, -5};
        elementStorePtr->store(node, range, *styleProviderPtr);
    }

    ~Index_InMemoryElementStoreFixture()
    {
        BOOST_TEST_MESSAGE("teardown fixture");
        delete elementStorePtr;
        delete styleProviderPtr;
        delete stringTablePtr;
        std::remove("string.idx");
        std::remove("string.dat");
    }

    StringTable* stringTablePtr;
    StyleProvider* styleProviderPtr;
    InMemoryElementStore* elementStorePtr;
};

struct ElementCounter : public ElementVisitor
{
    int times = 0;

    void visitNode(const Node&) { ++times; }
    void visitWay(const Way&) { ++times; }
    void visitArea(const Area&) { ++times; }
    void visitRelation(const Relation&)  { ++times; }
};

BOOST_FIXTURE_TEST_SUITE(Index_InMemoryElementStore, Index_InMemoryElementStoreFixture)

BOOST_AUTO_TEST_CASE(GivenNodeWayArea_WhenSearch_AllFound)
{
    QuadKey quadKey;
    quadKey.levelOfDetail = 1;
    quadKey.tileX = 0;
    quadKey.tileY = 0;
    ElementCounter counter;

    elementStorePtr->search(quadKey, *styleProviderPtr, counter);

    BOOST_CHECK_EQUAL(counter.times, 3);
}

BOOST_AUTO_TEST_CASE(GivenNodeWayArea_WhenSearch_AllSkipped)
{
    QuadKey quadKey;
    quadKey.levelOfDetail = 2;
    quadKey.tileX = 0;
    quadKey.tileY = 0;
    ElementCounter counter;

    elementStorePtr->search(quadKey, *styleProviderPtr, counter);

    BOOST_CHECK_EQUAL(counter.times, 0);
}

BOOST_AUTO_TEST_SUITE_END()