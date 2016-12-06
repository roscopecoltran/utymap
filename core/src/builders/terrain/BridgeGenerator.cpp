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

void BridgeGenerator::generate()
{
    buildForeground();

    context_.meshCallback(mesh_);
}

bool BridgeGenerator::canHandle(std::shared_ptr<Region> region)
{
    return false;
}

void BridgeGenerator::onAddRegion(const std::string& type, const utymap::entities::Element& element, const Style& style, std::shared_ptr<Region> region)
{
    // TODO
}

void BridgeGenerator::addGeometry(ClipperLib::Paths& geometry, const RegionContext& regionContext)
{
    // TODO
}
