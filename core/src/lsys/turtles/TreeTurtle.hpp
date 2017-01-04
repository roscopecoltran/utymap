#ifndef LSYS_TURTLES_TREETURTLE_HPP_DEFINED
#define LSYS_TURTLES_TREETURTLE_HPP_DEFINED

#include "builders/BuilderContext.hpp"
#include "builders/MeshContext.hpp"
#include "builders/generators/CylinderGenerator.hpp"
#include "builders/generators/IcoSphereGenerator.hpp"

#include "lsys/turtles/Turtle3d.hpp"

namespace utymap { namespace lsys {

/// Defines turtle which generates tree.
class TreeTurtle final: public Turtle3d
{
public:

    TreeTurtle(const utymap::builders::BuilderContext& builderContext,
               utymap::builders::MeshContext& meshContext);

    void moveForward() override;

    void say(const std::string& word) override;

private:

    void addLeaf();

    void addTrunk();

    utymap::builders::CylinderGenerator cylinderGenerator_;
    utymap::builders::IcoSphereGenerator icoSphereGenerator_;

    utymap::math::Vector3 trunkSize_ = utymap::math::Vector3::zero();
    utymap::math::Vector3 foliageSize_ = utymap::math::Vector3::zero();
};

}}

#endif // LSYS_TURTLES_TREETURTLE_HPP_DEFINED
