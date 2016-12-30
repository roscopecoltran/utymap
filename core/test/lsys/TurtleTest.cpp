#include "lsys/LSystem.hpp"
#include "lsys/Turtle.hpp"
#include "lsys/Rules.hpp"

#include <boost/test/unit_test.hpp>

using namespace utymap::lsys;

namespace {

    std::shared_ptr<MoveForwardRule> move() { return std::make_shared<MoveForwardRule>(); }
    std::shared_ptr<JumpForwardRule> jump() { return std::make_shared<JumpForwardRule>(); }

/// Defines turtle which writes its actions using common lsystem grammar notation.
class StringTurtle final : public Turtle
{
public:
    void moveForward() override  { path += "F"; }

    void jumpForward() override  { path += "f"; }

    std::string path;
};

}

BOOST_AUTO_TEST_SUITE(Lsys_Turtle)

BOOST_AUTO_TEST_CASE(GivenSimpleLSystemWithOneGeneration_Build_PerformsExpectedCommands)
{
    StringTurtle turtle;
    LSystem lsystem;
    lsystem.generations = 1;
    lsystem.angle = 1;
    lsystem.axiom.push_back(move());
    lsystem.productions[move()].push_back(std::make_pair<double, LSystem::Rules>(1, { move(), jump(), move() }));

    turtle.run(lsystem);

    BOOST_CHECK_EQUAL(turtle.path, "FfF");
}

BOOST_AUTO_TEST_CASE(GivenSimpleLSystemWithThreeGeneration_Build_PerformsExpectedCommands)
{
    StringTurtle turtle;
    LSystem lsystem;
    lsystem.generations = 3;
    lsystem.angle = 1;
    lsystem.axiom.push_back(move());
    lsystem.productions[move()].push_back(std::make_pair<double, LSystem::Rules>(1, { jump(), move(), jump(), jump() }));

    turtle.run(lsystem);

    BOOST_CHECK_EQUAL(turtle.path, "fffFffffff");
}

BOOST_AUTO_TEST_SUITE_END()
