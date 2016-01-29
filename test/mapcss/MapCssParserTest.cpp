#include "mapcss/StyleSheet.hpp"
#include "mapcss/MapCssParser.cpp"
#include "utils/CoreUtils.hpp"

#include <boost/test/unit_test.hpp>

#include <string>
#include <memory>

using namespace utymap::mapcss;

typedef std::string::const_iterator StringIterator;

struct MapCss_MapCssParserFixture {
    MapCss_MapCssParserFixture()        { BOOST_TEST_MESSAGE("setup fixture"); }
    ~MapCss_MapCssParserFixture()       { BOOST_TEST_MESSAGE("teardown fixture"); }

    // grammars
    CommentSkipper<StringIterator> skipper;
    StyleSheetGrammar<StringIterator> styleSheetGrammar;
    ConditionGrammar<StringIterator> conditionGrammar;
    ZoomGrammar<StringIterator> zoomGrammar;
    SelectorGrammar<StringIterator> selectorGrammar;
    DeclarationGrammar<StringIterator> declarationGrammar;
    RuleGrammar<StringIterator> ruleGrammar;
    // parsed data
    StyleSheet stylesheet;
    Condition condition;
    Zoom zoom;
    Selector selector;
    Declaration declaration;
    Rule rule;
};

BOOST_FIXTURE_TEST_SUITE( MapCss_MapCssParser, MapCss_MapCssParserFixture )

/* Comment */
BOOST_AUTO_TEST_CASE( GivenCppComment_WhenParse_ThenDoesNotBreak )
{
    std::string str = "/* Some \nncomment */\n way|z1-15[highway] {key:value;}";

    bool success = phrase_parse(str.cbegin(), str.cend(), styleSheetGrammar, skipper, stylesheet);

    BOOST_CHECK( success == true );
    BOOST_CHECK( stylesheet.rules.size() == 1 );
}

BOOST_AUTO_TEST_CASE( GivenHtmlComment_WhenParse_ThenDoesNotBreak )
{
    std::string str = "<!--Some \ncomment-->\n way|z1-15[highway] {key:value;}";

    bool success = phrase_parse(str.cbegin(), str.cend(), styleSheetGrammar, skipper, stylesheet);

    BOOST_CHECK( success == true );
    BOOST_CHECK( stylesheet.rules.size() == 1 );
}

/* Condition */
BOOST_AUTO_TEST_CASE( GivenExistenceCondition_WhenParse_ThenOnlyKeyIsSet )
{
    std::string str = "[highway]";

    bool success = phrase_parse(str.cbegin(), str.cend(), conditionGrammar, skipper, condition);

    BOOST_CHECK( success == true );
    BOOST_CHECK( condition.key == "highway" );
    BOOST_CHECK( condition.operation.empty() == true );
    BOOST_CHECK( condition.value.empty() == true );
}

BOOST_AUTO_TEST_CASE( GivenEqualCondition_WhenParse_ThenKeyOpValueAreSet )
{
    std::string str = "[highway=primary]";

    bool success = phrase_parse(str.cbegin(), str.cend(), conditionGrammar, skipper, condition);

    BOOST_CHECK( success == true );
    BOOST_CHECK( condition.key == "highway" );
    BOOST_CHECK( condition.operation == "=" );
    BOOST_CHECK( condition.value == "primary" );
}

BOOST_AUTO_TEST_CASE( GivenNegativeCondition_WhenParse_ThenKeyOpValueAreSet )
{
    std::string str = "[highway!=primary]";

    bool success = phrase_parse(str.cbegin(), str.cend(), conditionGrammar, skipper, condition);

    BOOST_CHECK( success == true );
    BOOST_CHECK( condition.key == "highway" );
    BOOST_CHECK( condition.operation == "!=" );
    BOOST_CHECK( condition.value == "primary" );
}

/* Zoom */
BOOST_AUTO_TEST_CASE( GivenOneZoom_WhenParse_ThenStartAndEndAreSet )
{
    std::string str = "|z1";

    bool success = phrase_parse(str.cbegin(), str.cend(), zoomGrammar, skipper, zoom);

    BOOST_CHECK( success == true );
    BOOST_CHECK( zoom.start == 1 );
    BOOST_CHECK( zoom.end == 1 );
}

BOOST_AUTO_TEST_CASE( GivenZoomRange_WhenParse_ThenStartAndEndAreSet )
{
    std::string str = "|z12-21";

    bool success = phrase_parse(str.cbegin(), str.cend(), zoomGrammar, skipper, zoom);

    BOOST_CHECK( success == true );
    BOOST_CHECK( zoom.start == 12 );
    BOOST_CHECK( zoom.end == 21 );
}

/* Selector */
BOOST_AUTO_TEST_CASE( GivenSingleSelector_WhenParse_ThenNameAndConditionsAreSet )
{
    std::string str = "way|z1[highway]";

    bool success = phrase_parse(str.cbegin(), str.cend(), selectorGrammar, skipper, selector);

    BOOST_CHECK( success == true );
    BOOST_CHECK( selector.name == "way" );
    BOOST_CHECK( selector.conditions.size() == 1 );
    BOOST_CHECK( selector.conditions[0].key == "highway" );
}

BOOST_AUTO_TEST_CASE( GivenTwoSelectors_WhenParse_ThenNameAndConditionsAreSet )
{
    std::string str = "way|z1[highway][landuse]";

    bool success = phrase_parse(str.cbegin(), str.cend(), selectorGrammar, skipper, selector);

    BOOST_CHECK( success == true );
    BOOST_CHECK( selector.name == "way" );
    BOOST_CHECK( selector.conditions.size() == 2 );
    BOOST_CHECK( selector.conditions[0].key == "highway" );
    BOOST_CHECK( selector.conditions[1].key == "landuse" );
}

/* Declaration */
BOOST_AUTO_TEST_CASE( GivenSingleDeclaraion_WhenParse_ThenKeyValueAreSet )
{
    std::string str = "key1:value1;";

    bool success = phrase_parse(str.cbegin(), str.cend(), declarationGrammar, skipper, declaration);

    BOOST_CHECK( success == true );
    BOOST_CHECK( declaration.key == "key1" );
    BOOST_CHECK( declaration.value == "value1" );
}

BOOST_AUTO_TEST_CASE(GivenGradientDeclaraion_WhenParse_ThenGradientValueIsCorrect)
{
    std::string str = "color:gradient(#dcdcdc 0%, #c0c0c0 10%, #a9a9a9 50%, #808080);";

    bool success = phrase_parse(str.cbegin(), str.cend(), declarationGrammar, skipper, declaration);

    BOOST_CHECK(success);
    BOOST_CHECK_EQUAL(declaration.key, "color");
    BOOST_CHECK_EQUAL(declaration.value, "gradient(#dcdcdc 0%, #c0c0c0 10%, #a9a9a9 50%, #808080)");
}

/* Rule */
BOOST_AUTO_TEST_CASE( GivenSimpleRule_WhenParse_ThenSelectorAndDeclarationAreSet )
{
    std::string str = "way|z1[highway] {key1:value1;}";

    bool success = phrase_parse(str.cbegin(), str.cend(), ruleGrammar, skipper, rule);

    BOOST_CHECK(success == true);

    BOOST_CHECK( rule.selectors.size() == 1 );
    BOOST_CHECK( rule.declarations.size() == 1 );
    BOOST_CHECK( rule.selectors[0].conditions[0].key == "highway" );
    BOOST_CHECK( rule.declarations[0].key == "key1" );
    BOOST_CHECK( rule.declarations[0].value == "value1" );
}

BOOST_AUTO_TEST_CASE( GivenComplexRule_WhenParse_ThenSelectorAndDeclarationAreSet )
{
    std::string str = "way|z1[highway],area|z2[landuse] { key1:value1; key2:value2; }";

    bool success = phrase_parse(str.cbegin(), str.cend(), ruleGrammar, skipper, rule);

    BOOST_CHECK(success == true);

    BOOST_CHECK( rule.selectors.size() == 2 );
    BOOST_CHECK( rule.declarations.size() == 2 );
    BOOST_CHECK( rule.selectors[0].name == "way" );
    BOOST_CHECK( rule.selectors[1].name == "area" );
    BOOST_CHECK( rule.selectors[0].conditions[0].key == "highway" );
    BOOST_CHECK( rule.selectors[1].conditions[0].key == "landuse" );
    BOOST_CHECK( rule.declarations[0].key == "key1" );
    BOOST_CHECK( rule.declarations[0].value == "value1" );
    BOOST_CHECK( rule.declarations[1].key == "key2" );
    BOOST_CHECK( rule.declarations[1].value == "value2" );
}

/* StyleSheet */
BOOST_AUTO_TEST_CASE( GivenFourRulesOnDifferentLines_WhenParse_ThenHasFourRules )
{
    std::string str =
        "way|z1[highway]  { key1:value1; }\n"
        "area|z2[landuse] { key2:value2; }\n"
        "node|z3[amenity] { key3:value3; }\n"
        "canvas|z3        { key4:value4; }";

    bool success = phrase_parse(str.cbegin(), str.cend(), styleSheetGrammar, skipper, stylesheet);

    BOOST_CHECK( success == true );
    BOOST_CHECK( stylesheet.rules.size() == 4) ;
}

BOOST_AUTO_TEST_CASE( GivenSimpleStyleSheet_WhenParserParse_ThenNoErrorsAndHasValidStyleSheet )
{
    std::string str = "way|z1[highway]  { key1:value1; }\n";
    Parser parser;

    StyleSheet stylesheet = parser.parse(str);

    BOOST_CHECK( parser.getError().empty() == true );
    BOOST_CHECK( stylesheet.rules.size() == 1 );
}

BOOST_AUTO_TEST_CASE(GivenCanvasRule_WhenParse_ThenProcessValidStyleSheet)
{
    std::string str = "canvas|z1 { key1:value1; }\n";
    Parser parser;

    StyleSheet stylesheet = parser.parse(str);

    BOOST_CHECK(parser.getError().empty() == true);
    BOOST_CHECK(stylesheet.rules.size() == 1);
}


BOOST_AUTO_TEST_CASE(GivenSimpleRuleWithZoomRange_WhenParse_ThenReturnCorrectZoomStartAndEnd)
{
    std::string str = "way|z1-12[highway]{key1:value1;}";
    Parser parser;

    StyleSheet stylesheet = parser.parse(str);

    BOOST_CHECK(parser.getError().empty());
    Selector selector = stylesheet.rules[0].selectors[0];
    BOOST_CHECK_EQUAL(1, selector.zoom.start);
    BOOST_CHECK_EQUAL(12, selector.zoom.end);
}

BOOST_AUTO_TEST_CASE(GivenRuleWithGradient_WhenParse_ThenReturnCorrectGradientValue)
{
    std::string str = "way|z1-12[highway]{color:gradient(#dcdcdc 0%, #c0c0c0 10%, #a9a9a9 50%, #808080);}";
    Parser parser;

    StyleSheet stylesheet = parser.parse(str);

    BOOST_CHECK(parser.getError().empty());
    BOOST_CHECK_EQUAL(1, stylesheet.rules.size());
    BOOST_CHECK_EQUAL(stylesheet.rules[0].declarations[0].value, "gradient(#dcdcdc 0%, #c0c0c0 10%, #a9a9a9 50%, #808080)");
}

BOOST_AUTO_TEST_CASE(GivenSimpleRule_WhenToString_ThenReturnCorrectRepresentation)
{
    std::string str = "way|z1-12[highway]{key1:value1;}";
    Parser parser;

    StyleSheet stylesheet = parser.parse(str);

    BOOST_CHECK(parser.getError().empty());
    BOOST_CHECK_EQUAL(1, stylesheet.rules.size());
    BOOST_CHECK_EQUAL(str, utymap::utils::toString(stylesheet.rules[0]));
}

BOOST_AUTO_TEST_SUITE_END()
