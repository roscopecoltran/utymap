#ifndef BUILDERS_BUILDINGS_ROOFS_LOWPOLYROOFBUILDER_HPP_DEFINED
#define BUILDERS_BUILDINGS_ROOFS_LOWPOLYROOFBUILDER_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "meshing/MeshTypes.hpp"
#include "meshing/MeshBuilder.hpp"
#include "meshing/Polygon.hpp"
#include "mapcss/ColorGradient.hpp"
#include "builders/buildings/roofs/RoofBuilder.hpp"

#include <vector>

namespace utymap { namespace builders {

// Builds flat roof in low poly.
class LowPolyFlatRoofBuilder : public RoofBuilder
{
public:
    LowPolyFlatRoofBuilder(utymap::meshing::Mesh& mesh,
                           const utymap::mapcss::ColorGradient& gradient,
                           const utymap::builders::BuilderContext& context)
         : RoofBuilder(mesh, gradient, context)
    {
    }

    void build(utymap::meshing::Polygon& polygon)
    {
        context_.meshBuilder.addPolygon(mesh_, polygon, utymap::meshing::MeshBuilder::Options
        {
            0, // area
            0, // ele noise
            colorNoiseFreq_,
            height_,
            gradient_,
            minHeight_
        });
    }
};

}}

#endif // BUILDERS_BUILDINGS_ROOFS_LOWPOLYROOFBUILDER_HPP_DEFINED
