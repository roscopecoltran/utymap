#include "lsys/LSystem.hpp"
#include "lsys/Turtle.hpp"
#include "lsys/Rules.hpp"

#include <boost/test/unit_test.hpp>

using namespace utymap::lsys;

namespace {

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
    lsystem.axiom.push_back(std::make_shared<MoveForwardRule>());
    lsystem.productions[std::make_shared<MoveForwardRule>()].push_back(std::make_pair<double, LSystem::Rules>(
            1,
            {
                std::make_shared<MoveForwardRule>(),
                std::make_shared<JumpForwardRule>(),
                std::make_shared<MoveForwardRule>()
            }));

    turtle.run(lsystem);

    assert(turtle.path == "FfF");
}

BOOST_AUTO_TEST_CASE(GivenSimpleLSystemWithThreeGeneration_Build_PerformsExpectedCommands)
{
    StringTurtle turtle;
    LSystem lsystem;
    lsystem.generations = 3;
    lsystem.angle = 1;
    lsystem.axiom.push_back(std::make_shared<MoveForwardRule>());
    lsystem.productions[std::make_shared<MoveForwardRule>()].push_back(std::make_pair<double, LSystem::Rules>(
        1,
        {
            std::make_shared<JumpForwardRule>(),
            std::make_shared<MoveForwardRule>(),
            std::make_shared<JumpForwardRule>(),
            std::make_shared<JumpForwardRule>()
        }));

    turtle.run(lsystem);

    assert(turtle.path == "fffFffffff");
}

BOOST_AUTO_TEST_SUITE_END()
