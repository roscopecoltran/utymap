#ifndef TERRAIN_MESHREGION_HPP_DEFINED
#define TERRAIN_MESHREGION_HPP_DEFINED

#include "meshing/MeshTypes.hpp"
#include <functional>
#include <vector>

namespace utymap { namespace terrain {

// Represents mesh region.
struct MeshRegion
{
    // Specifies properties of given region.
    struct Properties
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

        // Specific mesh action associated with given region.
        std::function<void(utymap::meshing::Mesh<double>)> action;

        Properties() :
            gradientKey(NoValue),
            textureAtlas(NoValue),
            textureKey(NoValue),
            eleNoiseFreq(0),
            colorNoiseFreq(0),
            heightOffset(0),
            action(nullptr)
        {
        }

        Properties& operator =(const Properties & obj)
        {
            if (this != &obj)
            {
                gradientKey = obj.gradientKey;
                textureAtlas = obj.textureAtlas;
                textureKey = obj.textureKey;
                eleNoiseFreq = obj.eleNoiseFreq;
                colorNoiseFreq = obj.colorNoiseFreq;
                heightOffset = obj.heightOffset;
                action = obj.action;
            }
            return *this;
        }
    };

    // Outer points.
    utymap::meshing::Contour<double> points;

    // Holes
    std::vector<utymap::meshing::Contour<double>> holes;

    // Properties of the region.
    Properties properties;
};

}}

#endif // TERRAIN_MESHREGION_HPP_DEFINED
