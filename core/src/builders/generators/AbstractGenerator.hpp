#ifndef BUILDERS_GENERATORS_ABSTRACTGENERATOR_HPP_DEFINED
#define BUILDERS_GENERATORS_ABSTRACTGENERATOR_HPP_DEFINED

#include "builders/BuilderContext.hpp"
#include "builders/MeshContext.hpp"
#include "meshing/MeshTypes.hpp"
#include "utils/NoiseUtils.hpp"

#include <algorithm>

namespace utymap { namespace builders {

/// Specifies basic behaviour of mesh generators.
class AbstractGenerator
{
public:

    AbstractGenerator(const utymap::builders::BuilderContext& builderContext,
                      utymap::builders::MeshContext& meshContext):
        builderContext_(builderContext),
        meshContext_(meshContext),
        vertNoiseFreq_(0)
    {
    }

    virtual ~AbstractGenerator() = default;

    /// Generates mesh data and updates given mesh.
    virtual void generate() = 0;

    /// Sets vertex noise frequency
    AbstractGenerator& setVertexNoiseFreq(double vertNoiseFreq)
    {
        vertNoiseFreq_ = vertNoiseFreq;
        return *this;
    }

    /// Sets color noise frequency
    AbstractGenerator& setColorNoiseFreq(double colorNoiseFreq)
    {
        meshContext_.appearanceOptions.colorNoiseFreq = colorNoiseFreq;
        return *this;
    }

protected:
    /// Adds triangle to mesh.
    void addTriangle(const utymap::meshing::Vector3& v0,
                     const utymap::meshing::Vector3& v1,
                     const utymap::meshing::Vector3& v2) const
    {
        // TODO check noise here.
        double noise = std::abs(vertNoiseFreq_) < 1E-5
                       ? utymap::utils::NoiseUtils::perlin2D(v0.x, v0.z, vertNoiseFreq_)
                       : 0;

        builderContext_.meshBuilder.addTriangle(meshContext_.mesh,
                                 utymap::meshing::Vector3(v0.x + noise, v0.y + noise, v0.z + noise),
                                 utymap::meshing::Vector3(v1.x + noise, v1.y + noise, v1.z + noise),
                                 utymap::meshing::Vector3(v2.x + noise, v2.y + noise, v2.z + noise),
                                 utymap::meshing::Vector2(0, 0),
                                 utymap::meshing::Vector2(1, 0),
                                 utymap::meshing::Vector2(1, 1),
                                 meshContext_.geometryOptions,
                                 meshContext_.appearanceOptions);
    }

    void addPlane(const utymap::meshing::Vector2& v0, 
                  const utymap::meshing::Vector2& v1,
                  double bottom,
                  double top) const
    {
        meshContext_.geometryOptions.elevation = bottom;
        meshContext_.geometryOptions.heightOffset = top - bottom;

        builderContext_.meshBuilder.addPlane(meshContext_.mesh,
                                    v0,
                                    v1,
                                    bottom,
                                    bottom,
                                    meshContext_.geometryOptions,
                                    meshContext_.appearanceOptions);
    }

private:
    const utymap::builders::BuilderContext& builderContext_;
    utymap::builders::MeshContext& meshContext_;
    double vertNoiseFreq_;
};

}}

#endif // BUILDERS_GENERATORS_ABSTRACTGENERATOR_HPP_DEFINED
