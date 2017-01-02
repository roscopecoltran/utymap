#include "lsys/turtles/Turtle3d.hpp"
#include "lsys/LSystem.hpp"
#include "utils/MathUtils.hpp"

using namespace utymap::lsys;
using namespace utymap::math;

Turtle3d::Turtle3d() :
    mesh_(""), states_()
{
}

void Turtle3d::turnLeft()
{
    auto rotation = createRotation(-state_.angle);
    state_.direction = rotation * state_.direction;
    state_.right = rotation * state_.right;
}

void Turtle3d::turnRight()
{
    auto rotation = createRotation(state_.angle);
    state_.direction = rotation * state_.direction;
    state_.right = rotation * state_.right;
}

void Turtle3d::turnAround()
{
    auto rotation = createRotation(utymap::utils::deg2Rad(180));
    state_.direction = rotation * state_.direction;
    state_.right = rotation * state_.right;
}

void Turtle3d::pitchUp()
{
    state_.direction = createRotation(state_.angle) * state_.direction;
}

void Turtle3d::pitchDown()
{
    state_.direction = createRotation(-state_.angle) * state_.direction;
}

void Turtle3d::rollLeft()
{
    state_.right = createRotation(-state_.angle) * state_.right;
}

void Turtle3d::rollRight()
{
    state_.right = createRotation(state_.angle) * state_.right;
}

void Turtle3d::increment()
{
}

void Turtle3d::decrement()
{
}

void Turtle3d::scaleUp()
{
}

void Turtle3d::scaleDown()
{
}

void Turtle3d::save()
{
    states_.push(state_);
}

void Turtle3d::restore()
{
    state_ = states_.top();
    states_.pop();
}

void Turtle3d::run(const LSystem& lsystem)
{
    state_.angle = utymap::utils::deg2Rad(lsystem.angle);
    state_.width = lsystem.width;
    state_.length = lsystem.length;

    Turtle::run(lsystem);
}

utymap::math::Quaternion Turtle3d::createRotation(double angle) const
{
    return Quaternion::fromAngleAxis(angle, Vector3::cross(state_.direction, state_.right));
}
