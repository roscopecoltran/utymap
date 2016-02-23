#ifndef SCENE_BUILDINGS_FACADES_LOWPOLYROOFBUILDER_HPP_DEFINED
#define SCENE_BUILDINGS_FACADES_LOWPOLYROOFBUILDER_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "meshing/MeshTypes.hpp"
#include "meshing/MeshBuilder.hpp"
#include "meshing/Polygon.hpp"
#include "mapcss/ColorGradient.hpp"
#include "scene/buildings/roofs/RoofBuilder.hpp"

#include <vector>

namespace utymap { namespace scene {

// Builds flat roof in low poly.
class LowPolyFlatRoofBuilder : public RoofBuilder
{
public:
    LowPolyFlatRoofBuilder(const utymap::meshing::Mesh& mesh,
                           const utymap::mapcss::ColorGradient& gradient,
                           const utymap::meshing::MeshBuilder& meshBuilder) 
        : RoofBuilder(mesh, gradient), meshBuilder_(meshBuilder)
    {
    }


    void build(utymap::meshing::Polygon& polygon)
    {
        meshBuilder_.build(polygon, utymap::meshing::MeshBuilder::Options
        {
            0, // area
            0, // ele noise
            colorNoiseFreq_,
            height_,
            gradient_,
            /* segmentSplit=*/ 0
        }, mesh_);
    }

private:
    const utymap::meshing::MeshBuilder& meshBuilder_;
};

}}

#endif // SCENE_BUILDINGS_FACADES_LOWPOLYROOFBUILDER_HPP_DEFINED
