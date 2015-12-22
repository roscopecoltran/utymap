#ifndef TERRAIN_TERRABUILDER_HPP_DEFINED
#define TERRAIN_TERRABUILDER_HPP_DEFINED

#include "meshing/MeshTypes.hpp"
#include "terrain/MeshRegion.hpp"

namespace utymap { namespace terrain {

// Provides the way to build tile.
class TileBuilder
{
public:
    // Adds water region to tile mesh.
    inline void addWater(const MeshRegion& water) 
    {
        waters_.push_back(water); 
    }

    // Adds surface region to tile mesh.
    inline void addSurface(const MeshRegion& surface) 
    {
        surfaces_.push_back(surface); 
    }

    // Add car road region to tile mesh.
    inline void addCarRoad(const MeshRegion& carRoad) 
    {
        carRoads_.push_back(carRoad);
    }

    // Add walk road region to tile mesh.
    inline void addWalkRoad(const MeshRegion& walkRoad) 
    {
        walkRoads_.push_back(walkRoad); 
    }

    // builds tile mesh using data provided.
    utymap::meshing::Mesh<double> build(utymap::meshing::Rectangle<double> tileRect);

private:
    std::vector<MeshRegion> waters_;
    std::vector<MeshRegion> surfaces_;
    std::vector<MeshRegion> carRoads_;
    std::vector<MeshRegion> walkRoads_;
};

}}

#endif // TERRAIN_TERRABUILDER_HPP_DEFINED
