#include "builders/terrain/BridgeGenerator.hpp"

using namespace ClipperLib;
using namespace utymap::builders;
using namespace utymap::mapcss;
using namespace utymap::math;

namespace {
    const std::string TerrainMeshName = "terrain_bridge";
}

BridgeGenerator::BridgeGenerator(const BuilderContext& context, const Style& style, const Path& tileRect) :
        TerraGenerator(context, style, tileRect), mesh_(TerrainMeshName)
{
}

void BridgeGenerator::addRegion(const std::string& type, const utymap::entities::Element& element, const Style& style, std::shared_ptr<Region> region)
{
    if (region->level > 0)
        layers_[type].push(region);
}

void BridgeGenerator::generate()
{
    buildForeground();

    context_.meshCallback(mesh_);
}

void BridgeGenerator::addGeometry(ClipperLib::Paths& geometry, const RegionContext& regionContext)
{
    // TODO
}
