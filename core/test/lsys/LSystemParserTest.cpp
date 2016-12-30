#include "lsys/LSystemParser.hpp"

#include <boost/test/unit_test.hpp>

using namespace utymap::lsys;

namespace {
    const std::string simpleGrammar = "generations: 3"
                                      "angle: 45"
                                      "scale: 1.2"
                                      "axiom: D F A f f F K";
}

BOOST_AUTO_TEST_SUITE(Lsys_LSystemParser)

BOOST_AUTO_TEST_CASE(GivenSimpleGrammar_WhenParse_ThenHasGenerations)
{
    BOOST_CHECK_EQUAL(LSystemParser().parse(simpleGrammar).generations, 3);
}

BOOST_AUTO_TEST_CASE(GivenSimpleGrammar_WhenParse_ThenHasAngle)
{
    BOOST_CHECK_EQUAL(LSystemParser().parse(simpleGrammar).angle, 45);
}

BOOST_AUTO_TEST_CASE(GivenSimpleGrammar_WhenParse_ThenHasScale)
{
    BOOST_CHECK_EQUAL(LSystemParser().parse(simpleGrammar).scale, 1.2);
}

BOOST_AUTO_TEST_CASE(GivenSimpleGrammar_WhenParse_ThenHasExpectedAmountOfAxiomRules)
{
    BOOST_CHECK_EQUAL(LSystemParser().parse(simpleGrammar).axiom.size(), 7);
}

BOOST_AUTO_TEST_CASE(GivenGrammarWithComment_WhenParse_ThenSkipsCommentLine)
{
    BOOST_CHECK_EQUAL(LSystemParser().parse("generations: 3"
                                            "#angle:35\n"
                                            "angle: 45"
                                            "#angle:55").angle, 45);
}

BOOST_AUTO_TEST_SUITE_END()
