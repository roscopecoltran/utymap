#ifndef BUILDERS_BUILDINGS_ROOFS_FLATROOFBUILDER_HPP_DEFINED
#define BUILDERS_BUILDINGS_ROOFS_FLATROOFBUILDER_HPP_DEFINED

#include "builders/buildings/roofs/RoofBuilder.hpp"
#include "meshing/MeshBuilder.hpp"
#include "meshing/Polygon.hpp"

#include <vector>

namespace utymap { namespace builders {

// Builds flat roof in low poly.
class FlatRoofBuilder : public RoofBuilder
{
public:
    FlatRoofBuilder(const utymap::builders::BuilderContext& builderContext,
                    utymap::builders::MeshContext& meshContext)
         : RoofBuilder(builderContext, meshContext)
    {
    }

    void build(utymap::meshing::Polygon& polygon)
    {
        builderContext_.meshBuilder
                       .addPolygon(meshContext_.mesh,
                                   polygon,
                                   utymap::meshing::MeshBuilder::Options
                                   {
                                       0, // area
                                       0, // ele noise
                                       colorNoiseFreq_,
                                       height_,
                                       getColorGradient(),
                                       minHeight_
                                   });
    }
};

}}

#endif // BUILDERS_BUILDINGS_ROOFS_FLATROOFBUILDER_HPP_DEFINED
