#ifndef BUILDERS_ABSTRACTGENERATOR_HPP_DEFINED
#define BUILDERS_ABSTRACTGENERATOR_HPP_DEFINED

#include "mapcss/ColorGradient.hpp"
#include "meshing/MeshBuilder.hpp"
#include "meshing/MeshTypes.hpp"
#include "utils/NoiseUtils.hpp"

#include <algorithm>

namespace utymap { namespace builders {

// Specifies basic behaviour of mesh generators.
class AbstractGenerator
{
public:

    AbstractGenerator(utymap::meshing::Mesh& mesh,
                      const utymap::meshing::MeshBuilder& meshBuilder_,
                      const utymap::mapcss::ColorGradient& gradient):
            meshBuilder_(meshBuilder_), vertNoiseFreq_(0), mesh_(mesh), options_(0, 0, 0, 0, gradient)
    {
    }
    // Generates mesh data and updates given mesh.
    virtual void generate() = 0;

    // Sets vertex noise frequency
    AbstractGenerator& setVertexNoiseFreq(float vertNoiseFreq)
    {
        vertNoiseFreq_ = vertNoiseFreq;
        return *this;
    }

    // Sets color noise frequency
    AbstractGenerator& setColorNoiseFreq(float colorNoiseFreq)
    {
        options_.colorNoiseFreq = colorNoiseFreq;
        return *this;
    }

protected:

    // Adds triangle to mesh.
    void addTriangle(const utymap::meshing::Vector3& v0,
                     const utymap::meshing::Vector3& v1,
                     const utymap::meshing::Vector3& v2,
                     bool hasBackSide = false) const
    {
        double noise = std::abs(vertNoiseFreq_) < 1E-5
                       ? utymap::utils::NoiseUtils::perlin2D(v0.x, v0.z, vertNoiseFreq_)
                       : 0;

        meshBuilder_.addTriangle(mesh_,
                                 utymap::meshing::Vector3(v0.x + noise, v0.y + noise, v0.z + noise),
                                 utymap::meshing::Vector3(v1.x + noise, v1.y + noise, v1.z + noise),
                                 utymap::meshing::Vector3(v2.x + noise, v2.y + noise, v2.z + noise),
                                 options_,
                                 hasBackSide);
    }

private:
    double vertNoiseFreq_;
    utymap::meshing::Mesh& mesh_;
    const utymap::meshing::MeshBuilder& meshBuilder_;
    utymap::meshing::MeshBuilder::Options options_;
};

}}

#endif // BUILDERS_ABSTRACTGENERATOR_HPP_DEFINED
