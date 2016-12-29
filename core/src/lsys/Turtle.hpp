#ifndef LSYS_TURTLE_HPP_DEFINED
#define LSYS_TURTLE_HPP_DEFINED

#include <functional>

namespace utymap { namespace lsys {

struct LSystem;

/// Defines abstract turtle behaviour.
class Turtle
{
public:
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
    void run(const LSystem& lsystem);
};

}}

#endif // LSYS_TURTLE_HPP_DEFINED
