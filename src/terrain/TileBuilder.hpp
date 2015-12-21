#ifndef TERRAIN_TILEBUILDER_HPP_DEFINED
#define TERRAIN_TILEBUILDER_HPP_DEFINED

#include "meshing/MeshTypes.hpp"
#include "terrain/MeshRegion.hpp"

namespace utymap { namespace terrain {

// Provides the way to build tile.
class TileBuilder
{
public:
    // Adds water region to tile mesh.
    void addWater(const MeshRegion& water);

    // Adds surface region to tile mesh.
    void addSurface(const MeshRegion& surface);

    // Add car road region to tile mesh.
    void addCarRoad(const MeshRegion& carRoad);

    // Add walk road region to tile mesh.
    void addWalkRoad(const MeshRegion& walkRoad);

    // builds tile mesh using data provided.
    utymap::meshing::Mesh<double> build(utymap::meshing::Rectangle<double> tileRect);

private:
    std::vector<MeshRegion> waters_;
    std::vector<MeshRegion> surfaces_;
    std::vector<MeshRegion> carRoads_;
    std::vector<MeshRegion> walkRoads_;
};

}}

#endif // TERRAIN_TILEBUILDER_HPP_DEFINED
