#ifndef TESTS_LSYS_STRINGTURTLE_HPP_DEFINED
#define TESTS_LSYS_STRINGTURTLE_HPP_DEFINED

#include "lsys/Turtle.hpp"

namespace utymap { namespace tests {

/// Defines turtle which writes its actions using common lsystem grammar notation.
class StringTurtle final : public utymap::lsys::Turtle
{
public:
    void moveForward() override  { path += "F"; }

    void jumpForward() override  { path += "f"; }

    std::string path;
};

}}

#endif // TESTS_LSYS_STRINGTURTLE_HPP_DEFINED
