#ifndef LSYS_TURTLES_TREETURTLE_HPP_DEFINED
#define LSYS_TURTLES_TREETURTLE_HPP_DEFINED

#include "lsys/Turtle.hpp"
#include "math/Mesh.hpp"
#include "math/Vector3.hpp"
#include "math/Quaternion.hpp"

#include <stack>

namespace utymap { namespace lsys {

/// Defines turtle for building trees.
class TreeTurtle final : public utymap::lsys::Turtle
{
    /// Defines turtle state.
    struct State
    {
        double angle;
        double length;
        double width;

        utymap::math::Vector3 position = utymap::math::Vector3::zero();
        utymap::math::Vector3 direction = utymap::math::Vector3::up();
        utymap::math::Vector3 right = utymap::math::Vector3::right();
    };

public:

    TreeTurtle();

    void moveForward() override;

    void jumpForward() override;

    void turnLeft() override;

    void turnRight() override;

    void turnAround() override;

    void pitchUp() override;

    void pitchDown() override;

    void rollLeft() override;

    void rollRight() override;

    void increment() override;

    void decrement() override;

    void scaleUp() override;

    void scaleDown() override;

    void save() override;

    void restore() override;

    void say(const std::string& word) override;

    void run(const LSystem& lsystem) override;
private:

    /// Creates rotation quaternion from current state.
    utymap::math::Quaternion createRotation(double angle) const;

    utymap::math::Mesh mesh_;
    State state_;
    std::stack<State> states_;
};

}}

#endif // LSYS_TURTLES_TREETURTLE_HPP_DEFINED
