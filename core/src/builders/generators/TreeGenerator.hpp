#ifndef BUILDERS_TREEGENERATOR_HPP_DEFINED
#define BUILDERS_TREEGENERATOR_HPP_DEFINED

#include "builders/generators/AbstractGenerator.hpp"
#include "builders/generators/CylinderGenerator.hpp"
#include "builders/generators/IcoSphereGenerator.hpp"
#include "meshing/MeshTypes.hpp"

#include <cmath>

namespace utymap { namespace builders {

// Generates tree.
class TreeGenerator : public AbstractGenerator
{
public:

    TreeGenerator(utymap::meshing::Mesh& mesh,
                  const utymap::meshing::MeshBuilder& meshBuilder,
                  const utymap::mapcss::ColorGradient& trunkGradient,
                  const utymap::mapcss::ColorGradient& foliageGradient) :
            AbstractGenerator(mesh, meshBuilder, foliageGradient), // ignored
            trunkGenerator(mesh, meshBuilder, trunkGradient),
            foliageGenerator(mesh, meshBuilder, foliageGradient),
            trunkHeight_(0), trunkRadius_(0), foliageRadius_(0), foliageHeight_(0)
    {
    }

    // Sets position of tree
    TreeGenerator& setPosition(const utymap::meshing::Vector3& position)
    {
        _position = position;
        return *this;
    }

    // Sets heigh of trunk
    TreeGenerator& setTrunkHeight(double height)
    {
        trunkHeight_ = height;
        return *this;
    }

    // Sets radius of trunk
    TreeGenerator& setTrunkRadius(double radius)
    {
        trunkRadius_ = radius;
        return *this;
    }

    // Sets radius of foliage
    TreeGenerator& setFoliageRadius(double radius, double height = 1)
    {
        foliageRadius_ = radius;
        foliageHeight_ = height;
        return *this;
    }

    void generate()
    {
        // generate trunk
        trunkGenerator
            .setCenter(_position)
            .setHeight(trunkHeight_)
            .setRadius(trunkRadius_)
            .setMaxSegmentHeight(5)
            .setRadialSegments(7)
            .setVertexNoiseFreq(0.1f)
            .generate();

        // generate foliage
        foliageGenerator
            .setCenter(utymap::meshing::Vector3(
                _position.x,
                _position.y + trunkHeight_ + foliageRadius_,
                _position.z))
            .setRadius(foliageRadius_, foliageHeight_)
            .setRecursionLevel(1)
            .setVertexNoiseFreq(0.1f)
            .generate();
    }

private:
    CylinderGenerator trunkGenerator;
    IcoSphereGenerator foliageGenerator;
    meshing::Vector3 _position;
    double trunkHeight_, trunkRadius_, foliageRadius_, foliageHeight_;
};
}}

#endif // BUILDERS_TREEGENERATOR_HPP_DEFINED
