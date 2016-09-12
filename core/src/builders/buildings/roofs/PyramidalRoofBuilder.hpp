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
        for (const auto& range : polygon.outers) {
            auto center = utymap::utils::getCentroid(polygon, range);
            auto options = utymap::meshing::MeshBuilder::Options(
                0, // area
                0, // ele noise
                colorNoiseFreq_,
                0,
                meshContext_.gradient,
                0);

            auto lastPointIndex = range.second - 2;

            for (std::size_t i = range.first; i < range.second; i += 2) {
                auto nextIndex = i == lastPointIndex ? range.first : i + 2;

                utymap::meshing::Vector3 v0(polygon.points[i], minHeight_, polygon.points[i + 1]);
                utymap::meshing::Vector3 v1(center.x, minHeight_ + height_, center.y);
                utymap::meshing::Vector3 v2(polygon.points[nextIndex], minHeight_, polygon.points[nextIndex + 1]);

                builderContext_.meshBuilder.addTriangle(meshContext_.mesh, v0, v1, v2, options, false);
            }
        }   
    }

private:

};

}}

#endif // BUILDERS_BUILDINGS_FACADES_PYRAMIDALROOFBUILDER_HPP_DEFINED
