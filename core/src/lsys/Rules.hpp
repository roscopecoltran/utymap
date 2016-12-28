#ifndef LSYS_RULES_HPP_DEFINED
#define LSYS_RULES_HPP_DEFINED

#include "LSystem.hpp"
#include "Turtle.hpp"

namespace utymap { namespace lsys {

class MoveForwardRule final : public Rule
{
public:
    void apply(Turtle& turtle) const override { turtle.moveForward(); }
};

class JumpForwardRule final : public Rule
{
public:
    void apply(Turtle& turtle) const override { turtle.jumpForward(); }
};

/// Defines rule for custom words.
class WordRule final : public Rule
{
public:
    WordRule(const std::string& word) : word_(word)
    {
    }

    void apply(Turtle& turtle) const override { turtle.say(word_); }

const std::string word_;
};

}}

#endif // LSYS_RULES_HPP_DEFINED
