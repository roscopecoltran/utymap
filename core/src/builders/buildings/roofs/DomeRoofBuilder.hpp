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
        utymap::GeoCoordinate center2d;
        double radius;

        // need to represent as vector of geocoordinates
        auto size = polygon.points.size() / 2;
        std::vector<utymap::GeoCoordinate> contour;
        contour.reserve(size);
        for (std::size_t i = 0; i < size; ++i) {
            contour.push_back(utymap::GeoCoordinate(polygon.points[i * 2 + 1], polygon.points[i * 2]));
        }

        utymap::utils::getCircle(contour, center2d, radius);

        utymap::builders::IcoSphereGenerator generator(builderContext_ ,
                                                       meshContext_,
                                                       RoofColorKey);

        double heightInMeters = utymap::utils::GeoUtils::distance(
            center2d, 
            utymap::GeoCoordinate(center2d.latitude + radius, center2d.longitude));

        generator
            .setCenter(utymap::meshing::Vector3(center2d.longitude, minHeight_, center2d.latitude))
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
