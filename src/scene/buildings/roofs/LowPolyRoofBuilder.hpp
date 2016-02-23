#ifndef SCENE_BUILDINGS_FACADES_LOWPOLYROOFBUILDER_HPP_DEFINED
#define SCENE_BUILDINGS_FACADES_LOWPOLYROOFBUILDER_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "meshing/MeshTypes.hpp"
#include "meshing/MeshBuilder.hpp"
#include "meshing/Polygon.hpp"
#include "mapcss/ColorGradient.hpp"

#include <vector>

namespace utymap { namespace scene {

// builds flat roof in low poly.
class LowPolyFlatRoofBuilder
{
public:
    LowPolyFlatRoofBuilder(const utymap::meshing::Mesh& mesh,
                           const utymap::mapcss::ColorGradient& gradient,
                           const utymap::meshing::MeshBuilder& meshBuilder)
        : mesh_(mesh), gradient_(gradient), meshBuilder_(meshBuilder)
    {
    }

    // Sets height.
    inline LowPolyFlatRoofBuilder& setHeight(double height) { height_ = height; return *this; }

    // Sets color freq.
    inline LowPolyFlatRoofBuilder& setColorNoise(double freq) { colorNoiseFreq_ = freq; return *this; }

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
    const utymap::meshing::Mesh& mesh_;
    const utymap::mapcss::ColorGradient& gradient_;
    const utymap::meshing::MeshBuilder& meshBuilder_;
    double height_, colorNoiseFreq_;
};

}}

#endif // SCENE_BUILDINGS_FACADES_LOWPOLYROOFBUILDER_HPP_DEFINED
