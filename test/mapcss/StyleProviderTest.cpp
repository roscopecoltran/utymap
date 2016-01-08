#include "entities/Element.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Relation.hpp"
#include "mapcss/StyleProvider.hpp"
#include "test_utils/MapCssUtils.hpp"
#include "test_utils/ElementUtils.hpp"

#include <boost/test/unit_test.hpp>

#include <cstdio>
#include <utility>

using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::mapcss;

struct Index_StyleProviderFixture
{
    Index_StyleProviderFixture() :
        tablePtr(new StringTable("")),
        stylesheetPtr(new StyleSheet()),
        styleProviderPtr(nullptr)
    {
        BOOST_TEST_MESSAGE("setup fixture");
        stylesheetPtr->rules.push_back(Rule());
    }

    ~Index_StyleProviderFixture()
    {
        BOOST_TEST_MESSAGE("teardown fixture");
        delete stylesheetPtr;
        delete tablePtr;
        delete styleProviderPtr;
        std::remove("string.idx");
        std::remove("string.dat");
    }

    void setSingleSelector(std::string name, int zoomStart, int zoomEnd,
        std::initializer_list<utymap::mapcss::Condition> conditions)
    {
        Selector selector;
        selector.name = name;
        selector.zoom.start = zoomStart;
        selector.zoom.end = zoomEnd;
        for (const utymap::mapcss::Condition& condition : conditions) {
            selector.conditions.push_back(condition);
        }
        stylesheetPtr->rules[0].selectors.push_back(selector);
        styleProviderPtr = new StyleProvider(*stylesheetPtr, *tablePtr);
    }

    StringTable* tablePtr;
    StyleSheet* stylesheetPtr;
    StyleProvider* styleProviderPtr;
};

BOOST_FIXTURE_TEST_SUITE(Index_StyleProvider, Index_StyleProviderFixture)

BOOST_AUTO_TEST_CASE(GivenSimpleEqualsCondition_WhenIsApplicable_ThenReturnTrue)
{
    int zoomLevel = 1;
    setSingleSelector("node", zoomLevel, zoomLevel, {{"amenity", "=", "biergarten"}});
    Node node = ElementUtils::createElement<Node>(*tablePtr,
    { 
        std::make_pair("amenity", "biergarten") 
    });

    bool result = styleProviderPtr->forElement(node, zoomLevel).isApplicable;

    BOOST_CHECK(result);
}

BOOST_AUTO_TEST_CASE(GivenSimpleEqualsConditionButDifferentZoomLevel_WhenIsApplicable_ThenReturnFalse)
{
    int zoomLevel = 1;
    setSingleSelector("node", zoomLevel, zoomLevel, {{"amenity", "=", "biergarten"}});
    Node node = ElementUtils::createElement<Node>(*tablePtr,
    { 
        std::make_pair("amenity", "biergarten") 
    });

    bool result = styleProviderPtr->forElement(node, 2).isApplicable;

    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_CASE(GivenTwoEqualsConditions_WhenIsApplicable_ThenReturnTrue)
{
    int zoomLevel = 1;
    setSingleSelector("node", zoomLevel, zoomLevel,
        {
            {"amenity", "=", "biergarten"},
            {"address", "=", "Invalidstr."}
        });
    Node node = ElementUtils::createElement<Node>(*tablePtr,
        {
            std::make_pair("amenity", "biergarten"),
            std::make_pair("address", "Invalidstr.")
        });

    bool result = styleProviderPtr->forElement(node, zoomLevel).isApplicable;

    BOOST_CHECK(result);
}

BOOST_AUTO_TEST_CASE(GivenTwoNotEqualsConditions_WhenIsApplicable_ThenReturnFalse)
{
    int zoomLevel = 1;
    setSingleSelector("node", zoomLevel, zoomLevel,
        {
            {"amenity", "=", "biergarten"},
            {"address", "!=", "Invalidstr."}
        });
    Node node = ElementUtils::createElement<Node>(*tablePtr,
        {
            std::make_pair("amenity", "biergarten"),
            std::make_pair("address", "Invalidstr.")
        });

    bool result = styleProviderPtr->forElement(node, zoomLevel).isApplicable;

    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_SUITE_END()
