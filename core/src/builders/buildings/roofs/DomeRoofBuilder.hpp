#ifndef SCENE_BUILDINGS_FACADES_LOWPOLYROOFBUILDER_HPP_DEFINED
#define SCENE_BUILDINGS_FACADES_LOWPOLYROOFBUILDER_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "builders/buildings/roofs/RoofBuilder.hpp"
#include "builders/generators/IcoSphereGenerator.hpp"
#include "utils/GeometryUtils.hpp"
#include "utils/GeoUtils.hpp"

#include <vector>

namespace utymap { namespace builders {

// Builds flat roof in low poly.
class DomeRoofBuilder : public RoofBuilder
{
public:
    DomeRoofBuilder(const utymap::builders::BuilderContext& builderContext,
                    utymap::builders::MeshContext& meshContext)
      : RoofBuilder(builderContext, meshContext)
    {
    }

    void build(utymap::meshing::Polygon& polygon)
    {
        utymap::meshing::Vector2 center2d;
        double radius;
        utymap::utils::getCircle(polygon.rectangle, center2d, radius);

        double heightInMeters = utymap::utils::GeoUtils::distance(
            utymap::GeoCoordinate(center2d.y, center2d.x),
            utymap::GeoCoordinate(center2d.y + radius, center2d.x));
        
        utymap::builders::IcoSphereGenerator generator(builderContext_, meshContext_, RoofColorKey);
        generator
            .setCenter(utymap::meshing::Vector3(center2d.x, minHeight_, center2d.y))
            .setRadius(radius, heightInMeters)
            .setRecursionLevel(2)
            .isSemiSphere(true)
            .setVertexNoiseFreq(0.0)
            .generate();
    }

private:

};

}}

#endif // SCENE_BUILDINGS_FACADES_LOWPOLYROOFBUILDER_HPP_DEFINED
