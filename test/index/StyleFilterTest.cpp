#include "index/StyleFilter.cpp"

#include <boost/test/unit_test.hpp>

#include <cstdio>
#include <initializer_list>
#include <utility>

using namespace utymap::index;
using namespace utymap::mapcss;

struct Index_StyleFilterFixture
{
    Index_StyleFilterFixture() :
        tablePtr(new StringTable("")),
        stylesheetPtr(new StyleSheet()),
        styleFilterPtr(nullptr)
    {
        BOOST_TEST_MESSAGE("setup fixture");
        stylesheetPtr->rules.push_back(Rule());
    }

    ~Index_StyleFilterFixture()
    {
        BOOST_TEST_MESSAGE("teardown fixture");
        delete stylesheetPtr;
        delete tablePtr;
        delete styleFilterPtr;
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
        styleFilterPtr = new StyleFilter(*stylesheetPtr, *tablePtr);
    }

    template <typename T>
    T createElement(std::initializer_list<std::pair<const char*, const char*>> tags)
    {
        T t;
        for (auto pair : tags) {
            uint32_t key = tablePtr->getId(pair.first);
            uint32_t value = tablePtr->getId(pair.second);
            Tag tag(key, value);
            t.tags.push_back(tag);
        }
        return t;
    }

    StringTable* tablePtr;
    StyleSheet* stylesheetPtr;
    StyleFilter* styleFilterPtr;
};

BOOST_FIXTURE_TEST_SUITE(Index_StyleFilter, Index_StyleFilterFixture)

BOOST_AUTO_TEST_CASE(GivenSimpleEqualsCondition_WhenIsApplicable_ThenReturnTrue)
{
    int zoomLevel = 1;
    setSingleSelector("node", zoomLevel, zoomLevel, {{"amenity", "=", "biergarten"}});
    Node node = createElement<Node>({ std::make_pair("amenity", "biergarten") });

    bool result = styleFilterPtr->get(node, zoomLevel).isApplicable;

    BOOST_CHECK(result);
}

BOOST_AUTO_TEST_CASE(GivenSimpleEqualsConditionButDifferentZoomLevel_WhenIsApplicable_ThenReturnFalse)
{
    int zoomLevel = 1;
    setSingleSelector("node", zoomLevel, zoomLevel, {{"amenity", "=", "biergarten"}});
    Node node = createElement<Node>({ std::make_pair("amenity", "biergarten") });

    bool result = styleFilterPtr->get(node, 2).isApplicable;

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
    Node node = createElement<Node>(
        {
            std::make_pair("amenity", "biergarten"),
            std::make_pair("address", "Invalidstr.")
        });

    bool result = styleFilterPtr->get(node, zoomLevel).isApplicable;

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
    Node node = createElement<Node>(
        {
            std::make_pair("amenity", "biergarten"),
            std::make_pair("address", "Invalidstr.")
        });

    bool result = styleFilterPtr->get(node, zoomLevel).isApplicable;

    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_SUITE_END()
