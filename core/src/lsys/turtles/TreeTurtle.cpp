#include "lsys/turtles/TreeTurtle.hpp"
#include "lsys/LSystem.hpp"
#include "utils/MathUtils.hpp"

using namespace utymap::lsys;
using namespace utymap::math;


TreeTurtle::TreeTurtle() :
    mesh_(""), states_()
{
}

void TreeTurtle::moveForward()
{
}

void TreeTurtle::jumpForward()
{
}

void TreeTurtle::turnLeft()
{
    auto rotation = createRotation(-state_.angle);
    state_.direction = rotation * state_.direction;
    state_.right = rotation * state_.right;
}

void TreeTurtle::turnRight()
{
    auto rotation = createRotation(state_.angle);
    state_.direction = rotation * state_.direction;
    state_.right = rotation * state_.right;
}

void TreeTurtle::turnAround()
{
    auto rotation = createRotation(utymap::utils::deg2Rad(180));
    state_.direction = rotation * state_.direction;
    state_.right = rotation * state_.right;
}

void TreeTurtle::pitchUp()
{
    state_.direction = createRotation(state_.angle) * state_.direction;
}

void TreeTurtle::pitchDown()
{
    state_.direction = createRotation(-state_.angle) * state_.direction;
}

void TreeTurtle::rollLeft()
{
    state_.right = createRotation(-state_.angle) * state_.right;
}

void TreeTurtle::rollRight()
{
    state_.right = createRotation(state_.angle) * state_.right;
}

void TreeTurtle::increment()
{
}

void TreeTurtle::decrement()
{
}

void TreeTurtle::scaleUp()
{
}

void TreeTurtle::scaleDown()
{
}

void TreeTurtle::save()
{
    states_.push(state_);
}

void TreeTurtle::restore()
{
    state_ = states_.top();
    states_.pop();
}

void TreeTurtle::say(const std::string& word)
{
}

void TreeTurtle::run(const LSystem& lsystem)
{
    state_.angle = utymap::utils::deg2Rad(lsystem.angle);
    state_.width = lsystem.width;
    state_.length = lsystem.length;

    Turtle::run(lsystem);
}

utymap::math::Quaternion TreeTurtle::createRotation(double angle) const
{
    return Quaternion::fromAngleAxis(angle, Vector3::cross(state_.direction, state_.right));
}
