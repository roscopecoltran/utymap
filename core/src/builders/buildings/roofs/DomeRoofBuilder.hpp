#ifndef SCENE_BUILDINGS_FACADES_LOWPOLYROOFBUILDER_HPP_DEFINED
#define SCENE_BUILDINGS_FACADES_LOWPOLYROOFBUILDER_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "builders/buildings/roofs/RoofBuilder.hpp"
#include "builders/generators/IcoSphereGenerator.hpp"
#include "utils/GeometryUtils.hpp"

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
        std::vector<utymap::GeoCoordinate> contour;
        contour.resize(polygon.points.size() / 2);
        for (auto i = 0; i < polygon.points.size(); i += 2) {
            contour.push_back(utymap::GeoCoordinate(polygon.points[i * 2], polygon.points[i * 2]));
        }

        utymap::utils::getCircle(contour, center2d, radius);

        utymap::meshing::Vector3 center3d(center2d.longitude, minHeight_, center2d.latitude);

        utymap::builders::IcoSphereGenerator generator(builderContext_ ,
                                                       meshContext_,
                                                       RoofColorKey);

        // TODO calculate height_
        generator
            .setCenter(center3d)
            .setRadius(radius, height_)
            .setRecursionLevel(2)
            .isSemiSphere(true)
            .setVertexNoiseFreq(0.1f)
            .generate();
    }

private:

};

}}

#endif // SCENE_BUILDINGS_FACADES_LOWPOLYROOFBUILDER_HPP_DEFINED
