#include <boost/test/unit_test.hpp>
#include "mapcss/Parser.hpp"
#include "mapcss/StyleSheet.hpp"

#include <string>
#include <memory>

using namespace utymap::mapcss;

BOOST_AUTO_TEST_SUITE( MapCss_Parser )

/**
    Checks whether stylesheet contains exactly one selector with expected data.
*/
void checkSimpleSelector(std::shared_ptr<StyleSheet> stylesheet,
                        std::string elementName,
                        std::string key = "",
                        std::string value = "")
{
    BOOST_CHECK(stylesheet != nullptr);
    BOOST_CHECK(stylesheet->rules_.size() == 1);
    SimpleSelector simpleSelector = stylesheet->rules_[0].selector_.simpleSelectors_[0];
    BOOST_CHECK(simpleSelector.elementName_ == elementName);
}

BOOST_AUTO_TEST_CASE( GivenMultilineComment_WhenParseIsCalled_ThenHasOneRule )
{
    std::string str = "/* Some comment */"
                      "way[highway] {}";
    Parser<std::string::iterator> parser;

    auto stylesheet = parser.parse(str.begin(), str.end());

    BOOST_CHECK(parser.getLastError().empty());
    checkSimpleSelector(stylesheet, "way");
}

BOOST_AUTO_TEST_CASE( GivenSimpleSelectorAndEmptyDeclarations_WhenParseIsCalled_ThenHasRule )
{
    std::string str = "way[highway] {}";
    Parser<std::string::iterator> parser;

    auto stylesheet = parser.parse(str.begin(), str.end());

    BOOST_CHECK(parser.getLastError().empty());
    checkSimpleSelector(stylesheet, "way");
}

BOOST_AUTO_TEST_CASE( GivenSimpleSelectorAndSingleDeclaration_WhenParseIsCalled_ThenHasCorrectRule )
{
    std::string str = "way[highway] {width: 3;}";
    Parser<std::string::iterator> parser;

    auto stylesheet = parser.parse(str.begin(), str.end());

    BOOST_CHECK(parser.getLastError().empty());
    checkSimpleSelector(stylesheet, "way");
}

BOOST_AUTO_TEST_SUITE_END()
