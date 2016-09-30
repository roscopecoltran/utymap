#ifndef BUILDERS_BUILDINGS_FACADES_DOMEROOFBUILDER_HPP_DEFINED
#define BUILDERS_BUILDINGS_FACADES_DOMEROOFBUILDER_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "builders/buildings/roofs/RoofBuilder.hpp"
#include "builders/generators/IcoSphereGenerator.hpp"
#include "utils/GeometryUtils.hpp"
#include "utils/GeoUtils.hpp"

namespace utymap { namespace builders {

/// Builds flat roof in low poly.
class DomeRoofBuilder : public RoofBuilder
{
public:
    DomeRoofBuilder(const utymap::builders::BuilderContext& builderContext,
                    utymap::builders::MeshContext& meshContext) :
        RoofBuilder(builderContext, meshContext)
    {
    }

    void build(utymap::meshing::Polygon& polygon) override
    {
        utymap::utils::outerRectangles(polygon, [&](const utymap::meshing::Rectangle& rectangle) {
            utymap::meshing::Vector2 center2d;
            double radius;
            utymap::utils::getCircle(rectangle, center2d, radius);

            double heightInMeters = utymap::utils::GeoUtils::distance(
                utymap::GeoCoordinate(center2d.y, center2d.x),
                utymap::GeoCoordinate(center2d.y + radius, center2d.x));

            utymap::builders::IcoSphereGenerator generator(builderContext_, meshContext_);
            generator
                .setCenter(utymap::meshing::Vector3(center2d.x, minHeight_, center2d.y))
                .setRadius(radius, heightInMeters)
                .setRecursionLevel(2)
                .isSemiSphere(true)
                .setVertexNoiseFreq(0.0)
                .setColorNoiseFreq(0)
                .generate();
        });
    }
};

}}

#endif // BUILDERS_BUILDINGS_FACADES_DOMEROOFBUILDER_HPP_DEFINED
