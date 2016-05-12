#include "entities/Node.hpp"
#include "mapcss/StyleDeclaration.hpp"

#include <boost/test/unit_test.hpp>

#include <cstdio>

#include "test_utils/DependencyProvider.hpp"
#include "test_utils/ElementUtils.hpp"

using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::mapcss;

struct Mapcss_StyleDeclarationFixture
{
    DependencyProvider dependencyProvider;
};

BOOST_FIXTURE_TEST_SUITE(Mapcss_StyleDeclaration, Mapcss_StyleDeclarationFixture)

BOOST_AUTO_TEST_CASE(GivenOnlySingleTag_WhenEvaluate_ReturnValue)
{
    StyleDeclaration styleDeclaration(0, "eval(\"tag('height')\")");

    double result = styleDeclaration.evaluate(
        ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), { { "height", "2.5" } }).tags,
        *dependencyProvider.getStringTable());

    BOOST_CHECK_EQUAL(result, 2.5);
}

BOOST_AUTO_TEST_CASE(GiveTwoTags_WhenEvaluate_ReturnValue)
{
    StyleDeclaration styleDeclaration(0, "eval(\"tag('building:height') - tag('roof:height')\")");

    double result = styleDeclaration.evaluate(ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(),
         { { "building:height", "10" }, { "roof:height", "2.5" } }).tags,
         *dependencyProvider.getStringTable());

    BOOST_CHECK_EQUAL(result, 7.5);
}

BOOST_AUTO_TEST_CASE(GiveOneTagOneNumber_WhenEvaluate_ReturnValue)
{
    StyleDeclaration styleDeclaration(0, "eval(\"tag('building:levels') * 3\")");

    double result = styleDeclaration.evaluate(ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(),
    { { "building:levels", "5" }}).tags,
    *dependencyProvider.getStringTable());

    BOOST_CHECK_EQUAL(result, 15);
}

BOOST_AUTO_TEST_CASE(GiveRawValue_WhenEvaluate_ReturnValue)
{
    StyleDeclaration styleDeclaration(0, "13");

    double result = styleDeclaration.evaluate(ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(),
    { { "building:levels", "5" } }).tags,
    *dependencyProvider.getStringTable());

    BOOST_CHECK_EQUAL(result, 13);
}

BOOST_AUTO_TEST_SUITE_END()
