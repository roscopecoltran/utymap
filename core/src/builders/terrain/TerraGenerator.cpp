#include "builders/terrain/TerraGenerator.hpp"

#include <functional>
#include <unordered_map>

using namespace ClipperLib;
using namespace utymap::builders;
using namespace utymap::mapcss;
using namespace utymap::meshing;
using namespace utymap::utils;

using namespace std::placeholders;

namespace {
    const double AreaTolerance = 1000; // Tolerance for meshing
    const double Scale = 1E7;

    const std::string TerrainMeshName = "terrain";
    const std::string ColorNoiseFreqKey = "color-noise-freq";
    const std::string EleNoiseFreqKey = "ele-noise-freq";
    const std::string GradientKey = "color";
    const std::string MaxAreaKey = "max-area";
    const std::string HeightOffsetKey = "height-offset";
    const std::string LayerPriorityKey = "layer-priority";
    const std::string MeshNameKey = "mesh-name";
    const std::string MeshExtrasKey = "mesh-extras";
    const std::string GridCellSize = "grid-cell-size";

    const std::unordered_map<std::string, TerraExtras::ExtrasFunc> ExtrasFuncs = 
    {
        { "forest", std::bind(&TerraExtras::addForest, _1, _2) },
        { "water", std::bind(&TerraExtras::addWater, _1, _2) },
    };
};

TerraGenerator::TerraGenerator(const BuilderContext& context, const Style& style, ClipperEx& foregroundClipper) :
    context_(context),
    style_(style),
    foregroundClipper_(foregroundClipper),
    backGroundClipper_(),
    mesh_(TerrainMeshName),
    rect_(context.boundingBox.minPoint.longitude,
            context.boundingBox.minPoint.latitude,
            context.boundingBox.maxPoint.longitude,
            context.boundingBox.maxPoint.latitude)
{
}

void TerraGenerator::addRegion(const std::string& type, std::unique_ptr<Region> region)
{
    layers_[type].push(std::move(region));
}

void TerraGenerator::generate(Path& tileRect)
{
    double size = style_.getValue(GridCellSize,
        context_.boundingBox.maxPoint.latitude - context_.boundingBox.minPoint.latitude, 
        context_.boundingBox.center());
    splitter_.setParams(Scale, size);

    buildLayers();
    buildBackground(tileRect);

    context_.meshCallback(mesh_);
}

// process all found layers.
void TerraGenerator::buildLayers()
{
    // 1. process layers: regions with shared properties.
    std::stringstream ss(style_.getString(LayerPriorityKey));
    while (ss.good()) {
        std::string name;
        getline(ss, name, ',');
        auto layer = layers_.find(name);
        if (layer != layers_.end()) {
            buildFromRegions(layer->second, createRegionContext(style_, name + "-"));
            layers_.erase(layer);
        }
    }

    // 2. Process the rest: each region has already its own properties.
    for (auto& layer : layers_)
        while (!layer.second.empty()) {
            auto& region = layer.second.top();
            buildFromPaths(region->points, *region->context);
            layer.second.pop();
        }
}

// process the rest area.
void TerraGenerator::buildBackground(Path& tileRect)
{
    backGroundClipper_.AddPath(tileRect, ptSubject, true);
    Paths background;
    backGroundClipper_.Execute(ctDifference, background, pftNonZero, pftNonZero);
    backGroundClipper_.Clear();

    if (!background.empty())
        populateMesh(background, createRegionContext(style_, ""));
}

TerraGenerator::RegionContext TerraGenerator::createRegionContext(const Style& style, const std::string& prefix) const
{
    double quadKeyWidth = context_.boundingBox.maxPoint.latitude - context_.boundingBox.minPoint.latitude;

    return TerraGenerator::RegionContext(style, prefix, MeshBuilder::Options(
        style.getValue(prefix + MaxAreaKey, quadKeyWidth * quadKeyWidth),
        style.getValue(prefix + EleNoiseFreqKey, quadKeyWidth),
        style.getValue(prefix + ColorNoiseFreqKey, quadKeyWidth),
        style.getValue(prefix + HeightOffsetKey, quadKeyWidth),
        context_.styleProvider.getGradient(style.getString(prefix + GradientKey)),
        std::numeric_limits<double>::lowest(), /* no elevation offset */ 
        false, /* don't flip */
        1 /* no new vertices on boundaries */));
}

void TerraGenerator::buildFromRegions(Regions& regions, const RegionContext& regionContext)
{
    // merge all regions together
    Clipper clipper;
    while (!regions.empty()) {
        clipper.AddPaths(regions.top()->points, ptSubject, true);
        regions.pop();
    }

    Paths result;
    clipper.Execute(ctUnion, result, pftNonZero, pftNonZero);

    buildFromPaths(result, regionContext);
}

void TerraGenerator::buildFromPaths(const Paths& paths, const RegionContext& regionContext)
{
    Paths solution;
    foregroundClipper_.AddPaths(paths, ptSubject, true);
    foregroundClipper_.Execute(ctDifference, solution, pftNonZero, pftNonZero);
    foregroundClipper_.moveSubjectToClip();

    populateMesh(solution, regionContext);
}

void TerraGenerator::populateMesh(Paths& paths, const RegionContext& regionContext)
{
    ClipperLib::SimplifyPolygons(paths);
    ClipperLib::CleanPolygons(paths);

    bool hasHeightOffset = std::abs(regionContext.options.heightOffset) > 1E-8;
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

        backGroundClipper_.AddPath(path, ptClip, true);

        Points points = restorePoints(path);
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

// restores mesh points from clipper points and injects new ones according to grid.
TerraGenerator::Points TerraGenerator::restorePoints(const Path& path) const
{
    auto lastItemIndex = path.size() - 1;
    Points points;
    points.reserve(path.size());
    for (int i = 0; i <= lastItemIndex; i++)
        splitter_.split(path[i], path[i == lastItemIndex ? 0 : i + 1], points);

    return std::move(points);
}

void TerraGenerator::fillMesh(Polygon& polygon, const RegionContext& regionContext)
{
    std::string meshName = regionContext.style.getString(regionContext.prefix + MeshNameKey);
    if (!meshName.empty()) {
        Mesh polygonMesh(meshName);
        TerraExtras::Context extrasContext(polygonMesh, regionContext.style);
        context_.meshBuilder.addPolygon(polygonMesh, polygon, regionContext.options);
        addExtrasIfNecessary(polygonMesh, extrasContext, regionContext);
        context_.meshCallback(polygonMesh);
    }
    else {
        TerraExtras::Context extrasContext(mesh_, regionContext.style);
        context_.meshBuilder.addPolygon(mesh_, polygon, regionContext.options);
        addExtrasIfNecessary(mesh_, extrasContext, regionContext);
    }
}

void TerraGenerator::addExtrasIfNecessary(utymap::meshing::Mesh& mesh,
                                          TerraExtras::Context& extrasContext,
                                          const RegionContext& regionContext) const
{
    std::string meshExtras = regionContext.style.getString(regionContext.prefix + MeshExtrasKey);
    if (meshExtras.empty())
        return;

    ExtrasFuncs.at(meshExtras)(context_, extrasContext);
}

void TerraGenerator::processHeightOffset(const Points& points, const RegionContext& regionContext)
{
    // do not use elevation noise for height offset.
    auto newOptions = regionContext.options;
    newOptions.eleNoiseFreq = 0;

    for (std::size_t i = 0; i < points.size(); ++i) {
        Vector2 p1 = points[i];
        Vector2 p2 = points[i == (points.size() - 1) ? 0 : i + 1];

        // check whether two points are on cell rect
        if (rect_.isOnBorder(p1) && rect_.isOnBorder(p2))
            continue;

        context_.meshBuilder.addPlane(mesh_, p1, p2, newOptions);
    }
}
