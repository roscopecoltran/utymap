#include "entities/Element.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Relation.hpp"
#include "mapcss/StyleProvider.hpp"
#include "test_utils/MapCssUtils.hpp"
#include "test_utils/ElementUtils.hpp"

#include <boost/test/unit_test.hpp>
#include "test_utils/DependencyProvider.hpp"

using namespace utymap::entities;
using namespace utymap::mapcss;

struct Index_StyleProviderFixture
{
    Index_StyleProviderFixture() :
        dependencyProvider(),
        stylesheet(new StyleSheet())
    {
        stylesheet->rules.push_back(Rule());
    }

    void setSingleSelector(const std::string& name, int zoomStart, int zoomEnd,
        const std::initializer_list<utymap::mapcss::Condition>& conditions)
    {
        Selector selector;
        selector.name = name;
        selector.zoom.start = zoomStart;
        selector.zoom.end = zoomEnd;
        for (const auto& condition : conditions) {
            selector.conditions.push_back(condition);
        }
        stylesheet->rules[0].selectors.push_back(selector);
        styleProvider = std::shared_ptr<StyleProvider>(new StyleProvider(*stylesheet, *dependencyProvider.getStringTable()));
    }

    DependencyProvider dependencyProvider;
    std::shared_ptr<StyleProvider> styleProvider;
    std::shared_ptr<StyleSheet> stylesheet;
};

BOOST_FIXTURE_TEST_SUITE(Index_StyleProvider, Index_StyleProviderFixture)

BOOST_AUTO_TEST_CASE(GivenSimpleEqualsCondition_WhenHasStyle_ThenReturnTrue)
{
    int zoomLevel = 1;
    setSingleSelector("node", zoomLevel, zoomLevel, {{"amenity", "=", "biergarten"}});
    Node node = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(),
    { 
        std::make_pair("amenity", "biergarten") 
    });

    BOOST_CHECK(styleProvider->hasStyle(node, zoomLevel));
}

BOOST_AUTO_TEST_CASE(GivenSimpleEqualsConditionButDifferentZoomLevel_WhenHasStyle_ThenReturnFalse)
{
    int zoomLevel = 1;
    setSingleSelector("node", zoomLevel, zoomLevel, {{"amenity", "=", "biergarten"}});
    Node node = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(),
    { 
        std::make_pair("amenity", "biergarten") 
    });

    BOOST_CHECK(!styleProvider->hasStyle(node, 2));
}

BOOST_AUTO_TEST_CASE(GivenTwoEqualsConditions_WhenHasStyle_ThenReturnTrue)
{
    int zoomLevel = 1;
    setSingleSelector("node", zoomLevel, zoomLevel,
        {
            {"amenity", "=", "biergarten"},
            {"address", "=", "Invalidstr."}
        });
    Node node = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(),
        {
            std::make_pair("amenity", "biergarten"),
            std::make_pair("address", "Invalidstr.")
        });

    BOOST_CHECK(styleProvider->hasStyle(node, zoomLevel));
}

BOOST_AUTO_TEST_CASE(GivenTwoNotEqualsConditions_WhenHasStyle_ThenReturnFalse)
{
    int zoomLevel = 1;
    setSingleSelector("node", zoomLevel, zoomLevel,
        {
            {"amenity", "=", "biergarten"},
            {"address", "!=", "Invalidstr."}
        });
    Node node = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(),
        {
            std::make_pair("amenity", "biergarten"),
            std::make_pair("address", "Invalidstr.")
        });

    BOOST_CHECK(!styleProvider->hasStyle(node, zoomLevel));
}

BOOST_AUTO_TEST_SUITE_END()
