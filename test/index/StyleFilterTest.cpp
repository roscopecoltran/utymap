#include "index/StyleFilter.cpp"

#include <boost/test/unit_test.hpp>

#include <initializer_list>
#include <utility>

using namespace utymap::index;
using namespace utymap::mapcss;

struct Index_StyleFilterFixture
{
    Index_StyleFilterFixture() :
        indexPath("index.idx"),
        stringPath("strings.dat"),
        table(*new StringTable(indexPath, stringPath)),
        stylesheet(*new StyleSheet())
    {
        BOOST_TEST_MESSAGE("setup fixture");
        stylesheet.rules.push_back(Rule());
    }

    ~Index_StyleFilterFixture()
    {
        BOOST_TEST_MESSAGE("teardown fixture");
        delete &stylesheet;
        delete &table;
        std::remove(indexPath.c_str());
        std::remove(stringPath.c_str());
    }

    std::string indexPath;
    std::string stringPath;
    StringTable& table;
    StyleSheet& stylesheet;
};

BOOST_FIXTURE_TEST_SUITE(Index_StyleFilter, Index_StyleFilterFixture)

void setAsSelector(StyleSheet& stylesheet, std::string name, 
    std::vector<utymap::mapcss::Condition>& conditions,
    int zoomStart = 1, int zoomEnd = 19)
{
    Selector selector;
    selector.name = name;
    selector.zoom.start = zoomStart;
    selector.zoom.end = zoomEnd;
    selector.conditions = conditions;
    stylesheet.rules[0].selectors.push_back(selector);
}

template <typename T>
T createElement(StringTable& table, std::initializer_list<std::pair<const char*, const char*>> tags)
{
    T t;
    for (auto pair : tags) {
        uint32_t key = table.getId(pair.first);
        uint32_t value = table.getId(pair.second);
        Tag tag(key, value);
        t.tags.push_back(tag);
    }
    return t;
}

BOOST_AUTO_TEST_CASE(GivenSimpleEqualsCondition_WhenIsApplicable_ThenReturnTrue)
{
    int zoomLevel = 1;
    std::vector<utymap::mapcss::Condition> conditions;
    utymap::mapcss::Condition condition = { "amenity", "=", "biergarten" };
    conditions.push_back(condition);
    setAsSelector(stylesheet, "node", conditions, zoomLevel, zoomLevel);
    StyleFilter filter(stylesheet, table);
    Node node = createElement<Node>(table, { std::make_pair("amenity", "biergarten") });

    bool result = filter.isApplicable(node, zoomLevel);

    BOOST_CHECK(result);
}

BOOST_AUTO_TEST_SUITE_END()