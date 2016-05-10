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
    DomeRoofBuilder(utymap::meshing::Mesh& mesh,
                    const utymap::mapcss::ColorGradient& gradient,
                    const utymap::builders::BuilderContext& context)
      : RoofBuilder(mesh, gradient, context)
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

        utymap::meshing::Vector3 center3d(center2d.longitude, minHeight_ + height_, center2d.latitude);

        utymap::builders::IcoSphereGenerator generator(mesh_, context_.meshBuilder, gradient_);
        generator
            .setCenter(center3d)
            .setRadius(radius, radius)
            .setRecursionLevel(1)
            .setVertexNoiseFreq(0.1f)
            .generate();
    }

private:

};

}}

#endif // SCENE_BUILDINGS_FACADES_LOWPOLYROOFBUILDER_HPP_DEFINED
