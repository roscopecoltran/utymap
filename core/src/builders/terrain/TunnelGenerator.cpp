#include "builders/terrain/TunnelGenerator.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "entities/ElementVisitor.hpp"

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

void TunnelGenerator::addRegion(const std::string& type, const utymap::entities::Element& element, const Style& style, std::shared_ptr<Region> region)
{
    if (region->level < 0)
        layers_[type].push(region);

    GeometryVisitor visitor(region->level);
    element.accept(visitor);
}

void TunnelGenerator::generate()
{
    buildForeground();

    context_.meshCallback(mesh_);
}

void TunnelGenerator::addGeometry(ClipperLib::Paths& geometry, const RegionContext& regionContext)
{
    // calculate approximate size of overall points
    double size = 0;
    for (std::size_t i = 0; i < geometry.size(); ++i)
        size += geometry[i].size() * 1.5;

    Polygon polygon(static_cast<std::size_t>(size));
    for (const Path& path : geometry) {
        double area = ClipperLib::Area(path);
        bool isHole = area < 0;
        if (std::abs(area) < AreaTolerance)
            continue;

        auto points = restoreGeometry(path);
        if (isHole)
            polygon.addHole(points);
        else
            polygon.addContour(points);
    }

    if (!polygon.points.empty())
        addGeometry(polygon, regionContext);
}

void TunnelGenerator::addGeometry(Polygon& polygon, const RegionContext& regionContext)
{
    context_.meshBuilder.addPolygon(mesh_, polygon, 
        regionContext.geometryOptions, regionContext.appearanceOptions);

    context_.meshBuilder.writeTextureMappingInfo(mesh_, regionContext.appearanceOptions);
}