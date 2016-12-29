#include "lsys/Turtle.hpp"
#include "lsys/Rules.hpp"

#include <boost/test/unit_test.hpp>

#include <random>

using namespace utymap::lsys;

namespace {

/// Defines turtle which writes its actions using common lsystem grammar notation.
class StringTurtle final : public Turtle
{
public:
    explicit StringTurtle(const Turtle::RuleSelector& selector) : Turtle(selector)
    {
    }

    void moveForward() override  { path += "F"; }

    void jumpForward() override  { path += "f"; }

    std::string path;
};

/// Randomization function.
const LSystem::Rules& selector(const LSystem::Productions& productions)
{
    std::vector<double> weights;
    for (const auto& p : productions)
        weights.push_back(p.first);

    std::random_device rd;
    std::mt19937 gen(rd());

    // NOTE replacement code in VS2013 for: std::discrete_distribution<> dist(first, last);
    auto first = weights.cbegin();
    auto count = std::distance(first, weights.cend());
    std::discrete_distribution<> dist(count, -0.5, -0.5 + count, [&first](size_t i) {
        return *std::next(first, i);
    });

    return productions.at(dist(gen)).second;
}

}

BOOST_AUTO_TEST_SUITE(Lsys_Turtle)

BOOST_AUTO_TEST_CASE(GivenSimpleLSystemWithOneGeneration_Build_PerformsExpectedCommands)
{
    StringTurtle turtle(selector);
    LSystem lsystem;
    lsystem.generations = 1;
    lsystem.angle = 1;
    lsystem.axiom.push_back(std::make_shared<MoveForwardRule>());
    lsystem.productions[std::type_index(typeid(MoveForwardRule))].push_back(std::make_pair<double, LSystem::Rules>(
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
    StringTurtle turtle(selector);
    LSystem lsystem;
    lsystem.generations = 3;
    lsystem.angle = 1;
    lsystem.axiom.push_back(std::make_shared<MoveForwardRule>());
    lsystem.productions[std::type_index(typeid(MoveForwardRule))].push_back(std::make_pair<double, LSystem::Rules>(
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
