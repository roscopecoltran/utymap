#include "builders/terrain/ExteriorGenerator.hpp"
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

const std::string TerrainMeshName = "terrain_exterior";

struct LineNetwork final
{
    
};

/// Visits geometry storing information for surface "touching" detection.
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

ExteriorGenerator::ExteriorGenerator(const BuilderContext& context, const Style& style, const Path& tileRect) :
    TerraGenerator(context, style, tileRect, TerrainMeshName)
{
}

void ExteriorGenerator::onNewRegion(const std::string& type, const utymap::entities::Element& element, const Style& style, const std::shared_ptr<Region>& region)
{
    GeometryVisitor visitor(region->level);
    element.accept(visitor);
}

void ExteriorGenerator::generateFrom(Layers& layers)
{
    for (const auto& layerPair : layers) {
        for (const auto& region : layerPair.second) {
            if (region->level < 0) {
                // TODO
                //TerraGenerator::addGeometry(region->geometry, *region->context, [](const Path& path) { });
            }
        }
    }

    context_.meshCallback(mesh_);
}

void ExteriorGenerator::addGeometry(utymap::math::Polygon& polygon, const RegionContext& regionContext)
{
    context_.meshBuilder.addPolygon(mesh_, polygon, 
        regionContext.geometryOptions, regionContext.appearanceOptions);
    context_.meshBuilder.writeTextureMappingInfo(mesh_, regionContext.appearanceOptions);
}
