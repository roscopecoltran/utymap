#include "builders/terrain/TunnelGenerator.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"

using namespace ClipperLib;
using namespace utymap::builders;
using namespace utymap::mapcss;
using namespace utymap::math;

namespace {

/// Tolerance for meshing
const double AreaTolerance = 1000;

const std::string TerrainMeshName = "terrain_tunnel";

struct LineNetwork final
{
    
};

struct GeometryVisitor : utymap::entities::ElementVisitor
{
    explicit GeometryVisitor(int level) : level_(level)
    {
    }

    void visitNode(const utymap::entities::Node&) override 
    {
    }

    void visitWay(const utymap::entities::Way& way) override
    {
    }

    void visitArea(const utymap::entities::Area& area) override
    {
    }

    void visitRelation(const utymap::entities::Relation& relation) override
    {
    }

private:
    int level_;
};

}

TunnelGenerator::TunnelGenerator(const BuilderContext& context, const Style& style, const Path& tileRect) :
    TerraGenerator(context, style, tileRect), mesh_(TerrainMeshName)
{
}

void TunnelGenerator::onAddRegion(const std::string& type, const utymap::entities::Element& element, const Style& style, const std::shared_ptr<Region>& region)
{
    GeometryVisitor visitor(region->level);
    element.accept(visitor);
}

void TunnelGenerator::generate()
{
    buildForeground();

    context_.meshCallback(mesh_);
}

bool TunnelGenerator::canHandle(const std::shared_ptr<Region>& region)
{
    return false;
}

void TunnelGenerator::addGeometry(ClipperLib::Paths& geometry, const RegionContext& regionContext)
{
    // TODO
}

void TunnelGenerator::addGeometry(Polygon& polygon, const RegionContext& regionContext)
{
    // TODO
}
