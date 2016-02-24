#ifndef SCENE_BUILDINGS_ROOFS_ROOFBUILDER_HPP_DEFINED
#define SCENE_BUILDINGS_ROOFS_ROOFBUILDER_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "meshing/MeshTypes.hpp"
#include "meshing/Polygon.hpp"
#include "mapcss/ColorGradient.hpp"

#include <vector>

namespace utymap { namespace builders {

// Specifies base roof builder functionality.
class RoofBuilder
{
public:
    RoofBuilder(utymap::meshing::Mesh& mesh, 
                const utymap::mapcss::ColorGradient& gradient)
                : mesh_(mesh), gradient_(gradient), colorNoiseFreq_(0)
    {
    }

    // Sets height.
    inline RoofBuilder& setHeight(double height) { height_ = height; return *this; }

    // Sets color freq.
    inline RoofBuilder& setColorNoise(double freq) { colorNoiseFreq_ = freq; return *this; }

    // Builds roof from polygon.
    virtual void build(utymap::meshing::Polygon& polygon) = 0;

protected:
    utymap::meshing::Mesh& mesh_;
    const utymap::mapcss::ColorGradient& gradient_;
    double height_, colorNoiseFreq_;
};

}}

#endif // SCENE_BUILDINGS_ROOFS_ROOFBUILDER_HPP_DEFINED
