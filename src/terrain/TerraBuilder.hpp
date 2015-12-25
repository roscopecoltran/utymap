#ifndef TERRAIN_TERRABUILDER_HPP_DEFINED
#define TERRAIN_TERRABUILDER_HPP_DEFINED

#include "heightmap/ElevationProvider.hpp"
#include "meshing/MeshTypes.hpp"
#include "terrain/MeshRegion.hpp"

#include <memory>

namespace utymap { namespace terrain {

// Provides the way to build tile.
class TerraBuilder
{
public:

    TerraBuilder(utymap::heightmap::ElevationProvider<double>& eleProvider);
    ~TerraBuilder();

    // Adds water region to tile mesh.
    void addWater(const MeshRegion& water);

    // Adds surface region to tile mesh. 
    // Regions will be sorted and merged using gradient key as reference.
    void addSurface(const MeshRegion& surface);

    // Add car road region to tile mesh.
    void addCarRoad(const MeshRegion& carRoad, int width);

    // Add walk road region to tile mesh.
    void addWalkRoad(const MeshRegion& walkRoad, int width);

    // Specifies background properties. Background region will be determined
    // by clipping operations.
    void setBackgroundProperties(const MeshRegion::Properties& properties);

    // builds tile mesh using data provided.
    utymap::meshing::Mesh<double> build(const utymap::meshing::Rectangle<double>& tileRect, int levelOfDetails);

private:
    class TerraBuilderImpl;
    std::unique_ptr<TerraBuilderImpl> pimpl_;
};

}}

#endif // TERRAIN_TERRABUILDER_HPP_DEFINED
