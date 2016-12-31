#include "lsys/LSystemParser.hpp"

#include <boost/test/unit_test.hpp>

using namespace utymap::lsys;

namespace {
    const std::string simpleGrammar = "generations: 3"
                                      "angle: 45"
                                      "scale: 1.2"
                                      "axiom: D F A f f F K\n"
                                      "F(0.3) -> F f\n"
                                      "f(0.5) -> F\n"
                                      "A -> f f F A";

    std::shared_ptr<MoveForwardRule> move() { return std::make_shared<MoveForwardRule>(); }
    std::shared_ptr<JumpForwardRule> jump() { return std::make_shared<JumpForwardRule>(); }
    std::shared_ptr<WordRule> A() { return std::make_shared<WordRule>("A"); }
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

BOOST_AUTO_TEST_CASE(GivenSimpleGrammar_WhenParse_ThenHasExpectedAmountOfProductions)
{
    auto lsystem = LSystemParser().parse(simpleGrammar);

    BOOST_CHECK_EQUAL(lsystem.productions.size(), 3);

    BOOST_CHECK_EQUAL(lsystem.productions[move()].size(), 1);
    BOOST_CHECK_EQUAL(lsystem.productions[move()].at(0).first, 0.3);
    BOOST_CHECK_EQUAL(lsystem.productions[move()].at(0).second.size(), 2);

    BOOST_CHECK_EQUAL(lsystem.productions[jump()].size(), 1);
    BOOST_CHECK_EQUAL(lsystem.productions[jump()].at(0).first, 0.5);
    BOOST_CHECK_EQUAL(lsystem.productions[jump()].at(0).second.size(), 1);

    BOOST_CHECK_EQUAL(lsystem.productions[A()].size(), 1);
    BOOST_CHECK_EQUAL(lsystem.productions[A()].at(0).first, 1);
    BOOST_CHECK_EQUAL(lsystem.productions[A()].at(0).second.size(), 4);
}

BOOST_AUTO_TEST_CASE(GivenGrammarWithComment_WhenParse_ThenSkipsCommentLine)
{
    const std::string grammar = "generations: 3"
                                "#angle:35\n"
                                "angle: 45"
                                "#angle:55\n"
                                "scale: 1.2"
                                "axiom: A\n"
                                "A -> f f";

    BOOST_CHECK_EQUAL(LSystemParser().parse(grammar).angle, 45);
}

BOOST_AUTO_TEST_SUITE_END()
