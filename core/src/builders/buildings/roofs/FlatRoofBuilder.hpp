#ifndef BUILDERS_BUILDINGS_ROOFS_FLATROOFBUILDER_HPP_DEFINED
#define BUILDERS_BUILDINGS_ROOFS_FLATROOFBUILDER_HPP_DEFINED

#include "builders/buildings/roofs/RoofBuilder.hpp"
#include "meshing/MeshBuilder.hpp"
#include "meshing/Polygon.hpp"

namespace utymap { namespace builders {

// Builds flat roof in low poly.
class FlatRoofBuilder : public RoofBuilder
{
public:
    FlatRoofBuilder(const utymap::builders::BuilderContext& builderContext,
                    utymap::builders::MeshContext& meshContext) : 
        RoofBuilder(builderContext, meshContext)
    {
    }

    FlatRoofBuilder& flipSide()
    {
        meshContext_.geometryOptions.flipSide = true;
        return *this;
    }

    void build(utymap::meshing::Polygon& polygon) override
    {
        meshContext_.geometryOptions.heightOffset = minHeight_;

        builderContext_.meshBuilder
                       .addPolygon(meshContext_.mesh,
                                   polygon,
                                   meshContext_.geometryOptions,
                                   meshContext_.appearanceOptions);
    }
};

}}

#endif // BUILDERS_BUILDINGS_ROOFS_FLATROOFBUILDER_HPP_DEFINED
