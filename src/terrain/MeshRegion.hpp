#ifndef TERRAIN_MESHREGION_HPP_DEFINED
#define TERRAIN_MESHREGION_HPP_DEFINED

#include "meshing/MeshTypes.hpp"
#include <functional>
#include <vector>

namespace utymap { namespace terrain {

// Represents mesh region.
struct MeshRegion
{
private:
    typedef std::vector<utymap::meshing::Point<double>> Contour;

public:
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

    // Outer points.
    Contour points;
    // Holes
    std::vector<Contour> holes;

    // Specific mesh action associated with given region.
    std::function<void(utymap::meshing::Mesh<double>)> action;
};

}}

#endif // TERRAIN_MESHREGION_HPP_DEFINED
