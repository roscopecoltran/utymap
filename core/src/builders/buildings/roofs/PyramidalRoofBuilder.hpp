#ifndef BUILDERS_BUILDINGS_FACADES_PYRAMIDALROOFBUILDER_HPP_DEFINED
#define BUILDERS_BUILDINGS_FACADES_PYRAMIDALROOFBUILDER_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "builders/buildings/roofs/RoofBuilder.hpp"
#include "utils/GeometryUtils.hpp"

#include <vector>

namespace utymap { namespace builders {

// Builds flat roof in low poly.
class PyramidalRoofBuilder : public RoofBuilder
{
public:
    PyramidalRoofBuilder(const utymap::builders::BuilderContext& builderContext,
                         utymap::builders::MeshContext& meshContext)
        : RoofBuilder(builderContext, meshContext)
    {
    }

    void build(utymap::meshing::Polygon& polygon)
    {
        auto center = utymap::utils::getCentroid(polygon);
        auto options = utymap::meshing::MeshBuilder::Options(
            0, // area
            0, // ele noise
            colorNoiseFreq_,
            0,
            gradient_,
            0
        );

        auto lastPointIndex = polygon.points.size() - 2;

        for (std::size_t i = 0; i < polygon.points.size(); i += 2) {
            auto nextIndex = i == lastPointIndex ? 0 : i + 2;

            utymap::meshing::Vector3 v0(polygon.points[i], minHeight_, polygon.points[i+1]);
            utymap::meshing::Vector3 v1(center.x, minHeight_ + height_, center.y);
            utymap::meshing::Vector3 v2(polygon.points[nextIndex], minHeight_, polygon.points[nextIndex + 1]);

            builderContext_.meshBuilder
                .addTriangle(meshContext_.mesh, v0, v1, v2, options, false);
        }
    }

private:

};

}}

#endif // BUILDERS_BUILDINGS_FACADES_PYRAMIDALROOFBUILDER_HPP_DEFINED
