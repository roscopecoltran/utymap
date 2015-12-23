#ifndef TERRAIN_MESHREGION_HPP_DEFINED
#define TERRAIN_MESHREGION_HPP_DEFINED

#include "meshing/MeshTypes.hpp"
#include <functional>
#include <vector>

namespace utymap { namespace terrain {

// Represents mesh region.
struct MeshRegion
{
    const int NoValue = -1;

    // Gradient key.
    int gradientKey;

    // Texture atlas.
    int textureAtlas;

    // Texture key.
    int textureKey;

    // Elevation noise frequency.
    float eleNoiseFreq;

    // Color noise frequency.
    float colorNoiseFreq;

    // Height offset of region.
    float heightOffset;

    // Outer points.
    utymap::meshing::Contour<double> points;

    // Holes
    std::vector<utymap::meshing::Contour<double>> holes;

    // Specific mesh action associated with given region.
    std::function<void(utymap::meshing::Mesh<double>)> action;

    MeshRegion() :
        gradientKey(NoValue),
        textureAtlas(NoValue),
        textureKey(NoValue),
        eleNoiseFreq(0),
        colorNoiseFreq(0),
        heightOffset(0),
        points(),
        holes(),
        action(nullptr)
    {
    }
};

}}

#endif // TERRAIN_MESHREGION_HPP_DEFINED
