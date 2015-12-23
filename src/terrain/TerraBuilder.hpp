#ifndef TERRAIN_TERRABUILDER_HPP_DEFINED
#define TERRAIN_TERRABUILDER_HPP_DEFINED

#include "meshing/MeshTypes.hpp"
#include "terrain/MeshRegion.hpp"

#include <map>
#include <unordered_map>
#include <vector>

namespace utymap { namespace terrain {

// Provides the way to build tile.
class TerraBuilder
{
public:
    // Adds water region to tile mesh.
    inline void addWater(const MeshRegion& water) 
    {
        waters_.push_back(water); 
    }

    // Adds surface region to tile mesh. 
    // Regions will be sorted and merged using gradient key as reference.
    inline void addSurface(const MeshRegion& surface) 
    {
        surfaces_[surface.gradientKey].push_back(surface);
    }

    // Add car road region to tile mesh.
    inline void addCarRoad(const MeshRegion& carRoad, int width) 
    {
        carRoads_[width].push_back(carRoad);
    }

    // Add walk road region to tile mesh.
    inline void addWalkRoad(const MeshRegion& walkRoad, int width) 
    {
        walkRoads_[width].push_back(walkRoad);
    }

    // builds tile mesh using data provided.
    utymap::meshing::Mesh<double> build(const utymap::meshing::Rectangle<double>& tileRect);

private:
    std::vector<MeshRegion> waters_;
    std::map<int, std::vector<MeshRegion>> surfaces_;
    std::unordered_map<int, std::vector<MeshRegion>> carRoads_;
    std::unordered_map<int, std::vector<MeshRegion>> walkRoads_;
};

}}

#endif // TERRAIN_TERRABUILDER_HPP_DEFINED
