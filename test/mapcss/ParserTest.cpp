#include <boost/test/unit_test.hpp>
#include "mapcss/Parser.hpp"
#include "mapcss/StyleSheet.hpp"

#include <string>
#include <memory>

using namespace utymap::mapcss;

BOOST_AUTO_TEST_SUITE( MapCss_Parser )

BOOST_AUTO_TEST_CASE( GivenMultilineComment_WhenParseIsCalled_ThenHasStylesheetAndNoError )
{
    std::string str = "/* Some comment */"
                      "way[highway] {}";
    Parser<std::string::iterator> parser;

    auto stylesheet = parser.parse(str.begin(), str.end());

    BOOST_CHECK(stylesheet != nullptr);
    BOOST_CHECK(parser.getLastError().empty());
}

BOOST_AUTO_TEST_CASE( GivenSimpleSelectorAndEmptyDeclarations_WhenParseIsCalled_ThenHasStylesheetAndNoError )
{
    std::string str = "way[highway] {}";
    Parser<std::string::iterator> parser;

    auto stylesheet = parser.parse(str.begin(), str.end());

    BOOST_CHECK(stylesheet != nullptr);
    BOOST_CHECK(parser.getLastError().empty());
}

BOOST_AUTO_TEST_CASE( GivenSimpleSelectorAndSingleDeclaration_WhenParseIsCalled_ThenHasStylesheetAndNoError )
{
    std::string str = "way[highway] {width: 3;}";
    Parser<std::string::iterator> parser;

    auto stylesheet = parser.parse(str.begin(), str.end());

    BOOST_CHECK(stylesheet != nullptr);
    BOOST_CHECK(parser.getLastError().empty());
}

BOOST_AUTO_TEST_SUITE_END()
