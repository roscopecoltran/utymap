#ifndef BUILDERS_GENERATORS_TREEGENERATOR_HPP_DEFINED
#define BUILDERS_GENERATORS_TREEGENERATOR_HPP_DEFINED

#include "builders/BuilderContext.hpp"
#include "builders/MeshContext.hpp"
#include "builders/generators/CylinderGenerator.hpp"
#include "builders/generators/IcoSphereGenerator.hpp"
#include "lsys/Turtle3d.hpp"

#include <functional>
#include <unordered_map>

namespace utymap { namespace builders {

/// Defines generator which generates a tree using lsystem.
class TreeGenerator final: public utymap::lsys::Turtle3d
{
    /// Maps word from lsystem grammar to the corresponding member function.
    static std::unordered_map<std::string, void(TreeGenerator::*)()> WordMap;
public:
    TreeGenerator(const utymap::builders::BuilderContext& builderContext,
                  const utymap::mapcss::Style& style,
                  utymap::math::Mesh& mesh);

    TreeGenerator& setPosition(const utymap::math::Vector3& position);

    void moveForward() override;

    void say(const std::string& word) override;

private:
    /// Adds leaf.
    void addLeaf();

    /// Adds trunk.
    void addTrunk();

    /// Adds cone.
    void addCone();

    const utymap::builders::BuilderContext& builderContext_;

    utymap::builders::MeshContext cylinderContext_;
    utymap::builders::MeshContext icoSphereContext_;

    utymap::builders::CylinderGenerator cylinderGenerator_;
    utymap::builders::IcoSphereGenerator icoSphereGenerator_;

    utymap::math::Vector3 trunkSize_ = utymap::math::Vector3::zero();
    utymap::math::Vector3 leafSize_ = utymap::math::Vector3::zero();
};

}}

#endif // BUILDERS_GENERATORS_TREEGENERATOR_HPP_DEFINED
