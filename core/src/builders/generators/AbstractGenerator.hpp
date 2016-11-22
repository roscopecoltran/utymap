#ifndef BUILDERS_GENERATORS_ABSTRACTGENERATOR_HPP_DEFINED
#define BUILDERS_GENERATORS_ABSTRACTGENERATOR_HPP_DEFINED

#include "builders/BuilderContext.hpp"
#include "builders/MeshContext.hpp"
#include "utils/NoiseUtils.hpp"

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
    void addTriangle(const utymap::math::Vector3& v0,
                     const utymap::math::Vector3& v1,
                     const utymap::math::Vector3& v2)
    {
        double noise = std::abs(vertNoiseFreq_) > 1E-5
                       ? utymap::utils::NoiseUtils::perlin2D(v0.x, v0.z, vertNoiseFreq_)
                       : 0;

        builderContext_.meshBuilder.addTriangle(meshContext_.mesh,
                                 utymap::math::Vector3(v0.x, v0.y + noise, v0.z),
                                 utymap::math::Vector3(v1.x, v1.y + noise, v1.z),
                                 utymap::math::Vector3(v2.x, v2.y + noise, v2.z),
                                 utymap::math::Vector2(0, 0),
                                 utymap::math::Vector2(1, 0),
                                 utymap::math::Vector2(1, 1),
                                 meshContext_.geometryOptions,
                                 meshContext_.appearanceOptions);
    }

    void addPlane(const utymap::math::Vector2& v0,
                  const utymap::math::Vector2& v1,
                  double bottom,
                  double top)
    {
        meshContext_.geometryOptions.elevation = bottom;
        meshContext_.geometryOptions.heightOffset = top - bottom;

        builderContext_.meshBuilder.addPlane(meshContext_.mesh,
                                    v0,
                                    v1,
                                    meshContext_.geometryOptions,
                                    meshContext_.appearanceOptions);
    }

    const utymap::builders::BuilderContext& builderContext_;
    utymap::builders::MeshContext& meshContext_;

private:
    double vertNoiseFreq_;
};

}}

#endif // BUILDERS_GENERATORS_ABSTRACTGENERATOR_HPP_DEFINED
