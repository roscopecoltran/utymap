#ifndef LSYS_TURTLE_HPP_DEFINED
#define LSYS_TURTLE_HPP_DEFINED

#include "LSystem.hpp"

#include <functional>

namespace utymap { namespace lsys {

/// Defines abstract turtle behaviour.
class Turtle
{
public:
    /// Specifies function type for selecting specific rules based on their probability.
    typedef std::function<const LSystem::Rules&(const LSystem::Productions&)> RuleSelector;

    explicit Turtle(const RuleSelector& selector) : selector_(selector)
    {
    }

    /// Moves forward emitting segment: F
    virtual void moveForward() {}

    /// Jumps forward without emitting segment: f
    virtual void jumpForward() {}

    /// Turns left: +
    virtual void turnLeft() {}

    /// Turns right: -
    virtual void turnRight() {}

    /// Performs instructions specific for given word.
    virtual void say(const std::string& word) {}

    /// Saves current state on stack.
    virtual void memorize() {}

    /// Restores current state from stack.
    virtual void restore() {}

    virtual ~Turtle() {}

    /// Runs turtle using lsystem provided.
    void run(const LSystem& lsystem)
    {
        // generate rules from axiom based on amount of generations and productions.
        auto rules = lsystem.axiom;
        for (int i = 0; i < lsystem.generations; ++i) {
            LSystem::Rules current;
            for (const auto& rule : rules) {
                const auto value = lsystem.productions.find(std::type_index(typeid(*rule)));
                if (value == lsystem.productions.end()) {
                    current.push_back(rule);
                    continue;
                }
                const auto& productions = selector_(value->second);
                current.insert(current.end(), productions.begin(), productions.end());
            }
            rules = std::move(current);
        }

        // visit all rules applying turtle's actions.
        for (const auto& rule : rules)
            rule->apply(*this);
    }

private:
    const RuleSelector selector_;
};

}}

#endif // LSYS_TURTLE_HPP_DEFINED
