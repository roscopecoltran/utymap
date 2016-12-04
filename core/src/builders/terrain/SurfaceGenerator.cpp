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
    TerraGenerator(context, style, tileRect),
    foregroundClipper_(),
    backgroundClipper_(),
    mesh_(TerrainMeshName),
    rect_(context.boundingBox.minPoint.longitude,
          context.boundingBox.minPoint.latitude,
          context.boundingBox.maxPoint.longitude,
          context.boundingBox.maxPoint.latitude)
{
}

void SurfaceGenerator::addRegion(const std::string& type, const utymap::entities::Element& element, const Style& style, std::shared_ptr<Region> region)
{
    if (region->level == 0)
        layers_[type].push(region);
}

void SurfaceGenerator::generate()
{
    backgroundClipper_.AddPath(tileRect_, ptSubject, true);

    double size = style_.getValue(StyleConsts::GridCellSize,
        context_.boundingBox.maxPoint.latitude - context_.boundingBox.minPoint.latitude, 
        context_.boundingBox.center());
    splitter_.setParams(Scale, size);

    buildLayers();
    buildBackground();

    context_.meshCallback(mesh_);
}

/// process all found layers.
void SurfaceGenerator::buildLayers()
{
    // 1. process layers: regions with shared properties.
    std::stringstream ss(style_.getString(StyleConsts::LayerPriorityKey));
    while (ss.good()) {
        std::string name;
        getline(ss, name, ',');
        auto layer = layers_.find(name);
        if (layer != layers_.end()) {
            buildFromRegions(layer->second, RegionContext::create(context_, style_, name + "-"));
            layers_.erase(layer);
        }
    }

    // 2. Process the rest: each region has already its own properties.
    for (auto& layer : layers_)
        while (!layer.second.empty()) {
            auto& region = layer.second.top();
            buildFromPaths(region->geometry, *region->context);
            layer.second.pop();
        }
}

/// process the rest area.
void SurfaceGenerator::buildBackground()
{
    Paths background;
    backgroundClipper_.Execute(ctDifference, background, pftNonZero, pftNonZero);
    backgroundClipper_.Clear();

    if (!background.empty())
        populateMesh(background, RegionContext::create(context_, style_, ""));
}

void SurfaceGenerator::buildFromRegions(Regions& regions, const RegionContext& regionContext)
{
    // merge all regions together
    Clipper clipper;
    while (!regions.empty()) {
        clipper.AddPaths(regions.top()->geometry, ptSubject, true);
        regions.pop();
    }

    Paths result;
    clipper.Execute(ctUnion, result, pftNonZero, pftNonZero);

    buildFromPaths(result, regionContext);
}

void SurfaceGenerator::buildFromPaths(const Paths& paths, const RegionContext& regionContext)
{
    Paths solution;
    foregroundClipper_.AddPaths(paths, ptSubject, true);
    foregroundClipper_.Execute(ctDifference, solution, pftNonZero, pftNonZero);
    foregroundClipper_.moveSubjectToClip();

    populateMesh(solution, regionContext);
}

void SurfaceGenerator::populateMesh(Paths& paths, const RegionContext& regionContext)
{
    ClipperLib::SimplifyPolygons(paths);
    ClipperLib::CleanPolygons(paths);

    bool hasHeightOffset = std::abs(regionContext.geometryOptions.heightOffset) > 0;
    // calculate approximate size of overall points
    double size = 0;
    for (std::size_t i = 0; i < paths.size(); ++i)
        size += paths[i].size() * 1.5;

    Polygon polygon(static_cast<std::size_t>(size));
    for (const Path& path : paths) {
        double area = ClipperLib::Area(path);
        bool isHole = area < 0;
        if (std::abs(area) < AreaTolerance)
            continue;

        backgroundClipper_.AddPath(path, ptClip, true);

        auto points = restorePoints(path);
        if (isHole)
            polygon.addHole(points);
        else
            polygon.addContour(points);

        if (hasHeightOffset)
            processHeightOffset(points, regionContext);
    }

    if (!polygon.points.empty())
        fillMesh(polygon, regionContext);
}

/// restores mesh points from clipper points and injects new ones according to grid.
std::vector<Vector2> SurfaceGenerator::restorePoints(const Path& path) const
{
    auto lastItemIndex = path.size() - 1;
    std::vector<utymap::math::Vector2> points;
    points.reserve(path.size());
    for (int i = 0; i <= lastItemIndex; i++)
        splitter_.split(path[i], path[i == lastItemIndex ? 0 : i + 1], points);

    return std::move(points);
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

void SurfaceGenerator::processHeightOffset(const std::vector<Vector2>& points, const RegionContext& regionContext)
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
