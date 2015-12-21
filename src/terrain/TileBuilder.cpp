#include "terrain/TileBuilder.hpp"

using namespace utymap::meshing;
using namespace utymap::terrain;

void TileBuilder::addWater(const MeshRegion& water)
{
    waters_.push_back(water);
}

void TileBuilder::addSurface(const MeshRegion& surface)
{
    surfaces_.push_back(surface);
}

void TileBuilder::addCarRoad(const MeshRegion& carRoad)
{
    carRoads_.push_back(carRoad);
}

void TileBuilder::addWalkRoad(const MeshRegion& walkRoad)
{
    walkRoads_.push_back(walkRoad);
}

Mesh<double> TileBuilder::build(Rectangle<double> tileRect)
{
    Mesh<double> mesh;

    return std::move(mesh);
}
