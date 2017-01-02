#ifndef LSYS_TURTLES_TURTLE3D_HPP_DEFINED
#define LSYS_TURTLES_TURTLE3D_HPP_DEFINED

#include "lsys/Turtle.hpp"
#include "math/Vector3.hpp"
#include "math/Quaternion.hpp"

#include <stack>

namespace utymap { namespace lsys {

/// Defines turtle which executes commands in 3d space.
class Turtle3d final : public utymap::lsys::Turtle
{
public:

    Turtle3d();

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

    virtual void run(const LSystem& lsystem) override;

protected:
    /// Defines turtle state.
    struct State
    {
        double length;
        double width;

        utymap::math::Vector3 position = utymap::math::Vector3::zero();
        utymap::math::Vector3 direction = utymap::math::Vector3::up();
        utymap::math::Vector3 right = utymap::math::Vector3::right();
    };

    State state_;

private:
    /// Creates rotation quaternion from current state.
    utymap::math::Quaternion createRotation(double angle) const;

    /// Angle defined by lsystem.
    double angle_= 0;
    /// Length scale defined by lsystem.
    double lengthScale_ = 1;
    /// Width step defined by lsystem.
    double widthStep_ = 0;
    /// Stack for storing states.
    std::stack<State> states_;
};

}}

#endif // LSYS_TURTLES_TURTLE3D_HPP_DEFINED
