#ifndef BUILDERS_BUILDINGS_FACADES_SPHEREFACADEBUILDER_HPP_DEFINED
#define BUILDERS_BUILDINGS_FACADES_SPHEREFACADEBUILDER_HPP_DEFINED

#include "builders/buildings/facades/FacadeBuilder.hpp"
#include "builders/generators/IcoSphereGenerator.hpp"
#include "meshing/MeshTypes.hpp"
#include "utils/GeometryUtils.hpp"

namespace utymap { namespace builders {

/// Responsible for building cylinder shaped facade.
class SphereFacadeBuilder : public FacadeBuilder
{
public:
    SphereFacadeBuilder(const utymap::builders::BuilderContext& builderContext,
                        utymap::builders::MeshContext& meshContext) : 
        FacadeBuilder(builderContext, meshContext)
    {
    }

    void build(utymap::meshing::Polygon& polygon) override
    {
        utymap::meshing::Vector2 center2d;
        double radius;
        utymap::utils::getCircle(polygon.rectangle, center2d, radius);

        double heightInMeters = utymap::utils::GeoUtils::distance(
                utymap::GeoCoordinate(center2d.y, center2d.x),
                utymap::GeoCoordinate(center2d.y + radius, center2d.x));

        IcoSphereGenerator icoSphereGenerator(builderContext_, meshContext_);
        icoSphereGenerator
                .setCenter(utymap::meshing::Vector3(center2d.x, minHeight_, center2d.y))
                .setRadius(radius, heightInMeters)
                .setRecursionLevel(2)
                .isSemiSphere(false)
                .setColorNoiseFreq(0)
                .generate();
    }
};

}}

#endif // BUILDERS_BUILDINGS_FACADES_SPHEREFACADEBUILDER_HPP_DEFINED
