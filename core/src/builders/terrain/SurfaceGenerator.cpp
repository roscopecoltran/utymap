#include "builders/terrain/SurfaceGenerator.hpp"
#include "math/Mesh.hpp"
#include "math/Vector2.hpp"

using namespace ClipperLib;
using namespace utymap::builders;
using namespace utymap::mapcss;
using namespace utymap::math;
using namespace utymap::utils;

using namespace std::placeholders;

namespace {
    /// Tolerance for meshing
    const double AreaTolerance = 1000;
    const double Scale = 1E7;

    const std::string TerrainMeshName = "terrain_surface";

    const std::unordered_map<std::string, TerraExtras::ExtrasFunc> ExtrasFuncs = 
    {
        { "forest", std::bind(&TerraExtras::addForest, _1, _2) },
        { "water", std::bind(&TerraExtras::addWater, _1, _2) },
    };
};

SurfaceGenerator::SurfaceGenerator(const BuilderContext& context, const Style& style, const ClipperLib::Path& tileRect) :
    TerraGenerator(context, style, tileRect), mesh_(TerrainMeshName)
{
}

void SurfaceGenerator::addRegion(const std::string& type, const utymap::entities::Element& element, const Style& style, std::shared_ptr<Region> region)
{
    if (region->level == 0)
        layers_[type].push(region);
}

void SurfaceGenerator::generate()
{
    double size = style_.getValue(StyleConsts::GridCellSize,
        context_.boundingBox.maxPoint.latitude - context_.boundingBox.minPoint.latitude, 
        context_.boundingBox.center());
    splitter_.setParams(Scale, size);

    buildLayers();
    buildBackground();

    context_.meshCallback(mesh_);
}

void SurfaceGenerator::addGeometry(Paths& geometry, const RegionContext& regionContext)
{
    ClipperLib::SimplifyPolygons(geometry);
    ClipperLib::CleanPolygons(geometry);

    bool hasHeightOffset = std::abs(regionContext.geometryOptions.heightOffset) > 0;
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

        backgroundClipper_.AddPath(path, ptClip, true);

        auto points = restoreGeometry(path);
        if (isHole)
            polygon.addHole(points);
        else
            polygon.addContour(points);

        if (hasHeightOffset)
            buildHeightOffset(points, regionContext);
    }

    if (!polygon.points.empty())
        fillMesh(polygon, regionContext);
}

void SurfaceGenerator::buildHeightOffset(const std::vector<Vector2>& points, const RegionContext& regionContext)
{
    // do not use elevation noise for height offset.
    auto newGeometryOptions = regionContext.geometryOptions;
    newGeometryOptions.eleNoiseFreq = 0;

    for (std::size_t i = 0; i < points.size(); ++i) {
        const auto& p1 = points[i];
        const auto& p2 = points[i == (points.size() - 1) ? 0 : i + 1];

        // check whether two points are on cell rect
        if (rect_.isOnBorder(p1) && rect_.isOnBorder(p2))
            continue;

        context_.meshBuilder.addPlane(mesh_, p1, p2, newGeometryOptions, regionContext.appearanceOptions);
    }
}

void SurfaceGenerator::fillMesh(Polygon& polygon, const RegionContext& regionContext)
{
    std::string meshName = regionContext.style.getString(regionContext.prefix + StyleConsts::MeshNameKey);
    if (!meshName.empty()) {
        Mesh polygonMesh(meshName);
        TerraExtras::Context extrasContext(polygonMesh, regionContext.style);
        context_.meshBuilder.addPolygon(polygonMesh, 
                                        polygon, 
                                        regionContext.geometryOptions, 
                                        regionContext.appearanceOptions);
        context_.meshBuilder.writeTextureMappingInfo(polygonMesh, regionContext.appearanceOptions);

        addExtrasIfNecessary(polygonMesh, extrasContext, regionContext);
        context_.meshCallback(polygonMesh);
    }
    else {
        TerraExtras::Context extrasContext(mesh_, regionContext.style);
        context_.meshBuilder.addPolygon(mesh_,
                                        polygon, 
                                        regionContext.geometryOptions, 
                                        regionContext.appearanceOptions);
        context_.meshBuilder.writeTextureMappingInfo(mesh_, regionContext.appearanceOptions);

        addExtrasIfNecessary(mesh_, extrasContext, regionContext);
    }
}

void SurfaceGenerator::addExtrasIfNecessary(Mesh& mesh,
                                            TerraExtras::Context& extrasContext,
                                            const RegionContext& regionContext) const
{
    std::string meshExtras = regionContext.style.getString(regionContext.prefix + StyleConsts::MeshExtrasKey);
    if (meshExtras.empty())
        return;

    ExtrasFuncs.at(meshExtras)(context_, extrasContext);
}
