#include "builders/terrain/SurfaceGenerator.hpp"

using namespace ClipperLib;
using namespace utymap::builders;
using namespace utymap::mapcss;
using namespace utymap::math;
using namespace utymap::utils;

using namespace std::placeholders;

namespace {
    /// Tolerance for meshing
    const double AreaTolerance = 1000;

    const std::string TerrainMeshName = "terrain_surface";

    const std::unordered_map<std::string, TerraExtras::ExtrasFunc> ExtrasFuncs = 
    {
        { "forest", std::bind(&TerraExtras::addForest, _1, _2) },
        { "water", std::bind(&TerraExtras::addWater, _1, _2) },
    };
};

SurfaceGenerator::SurfaceGenerator(const BuilderContext& context, const Style& style, const Path& tileRect) :
TerraGenerator(context, style, tileRect, TerrainMeshName)
{
}

void SurfaceGenerator::onNewRegion(const std::string& type, const utymap::entities::Element& element, const Style& style, const std::shared_ptr<Region>& region)
{
}

/// NOTE Original layer collection is cleared after this function executed.
void SurfaceGenerator::generateFrom(Layers& layers)
{
    buildForeground(layers);

    buildBackground();

    context_.meshCallback(mesh_);
}

void SurfaceGenerator::buildForeground(Layers& layers)
{
    // 1. Process layers according their priority
    std::stringstream ss(style_.getString(StyleConsts::LayerPriorityKey));
    while (ss.good()) {
        std::string name;
        getline(ss, name, ',');
        auto layer = layers.find(name);
        if (layer != layers.end()) {
            buildLayer(layer->second);
            layers.erase(layer);
        }
    }

    // 2. Process the rest.
    while (!layers.empty()) {
        auto layer = layers.begin();
        buildLayer(layer->second);
        layers.erase(layer->first);
    }
}

void SurfaceGenerator::buildBackground()
{
    Paths background;
    backgroundClipper_.AddPath(tileRect_, ptSubject, true);
    backgroundClipper_.Execute(ctDifference, background, pftNonZero, pftNonZero);
    backgroundClipper_.Clear();

    if (!background.empty())
        addGeometry(background, RegionContext::create(context_, style_, ""));
}

void SurfaceGenerator::buildLayer(Layer& layer)
{
    while (!layer.empty()) {
        auto& region = layer.top();
        if (region->level == 0)
            buildFromPaths(region->geometry, *region->context);
        
        layer.pop();
    }
}

void SurfaceGenerator::buildFromPaths(const Paths& paths, const RegionContext& regionContext)
{
    Paths solution;
    foregroundClipper_.AddPaths(paths, ptSubject, true);
    foregroundClipper_.Execute(ctDifference, solution, pftNonZero, pftNonZero);
    foregroundClipper_.moveSubjectToClip();

    ClipperLib::SimplifyPolygons(solution);
    ClipperLib::CleanPolygons(solution);

    addGeometry(solution, regionContext);
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
        if (isOnBorder(p1) && isOnBorder(p2))
            continue;

        context_.meshBuilder.addPlane(mesh_, p1, p2, newGeometryOptions, regionContext.appearanceOptions);
    }
}

void SurfaceGenerator::addGeometry(Paths& geometry, const RegionContext& regionContext)
{
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
        addGeometry(polygon, regionContext);
}

void SurfaceGenerator::addGeometry(Polygon& polygon, const RegionContext& regionContext)
{
    std::string meshName = regionContext.style.getString(regionContext.prefix + StyleConsts::MeshNameKey);
    if (!meshName.empty()) {
        Mesh polygonMesh(meshName);
        context_.meshBuilder.addPolygon(polygonMesh, polygon, 
            regionContext.geometryOptions, regionContext.appearanceOptions);
        context_.meshBuilder.writeTextureMappingInfo(polygonMesh, regionContext.appearanceOptions);

        TerraExtras::Context extrasContext(polygonMesh, regionContext.style);
        addExtrasIfNecessary(polygonMesh, extrasContext, regionContext);
        context_.meshCallback(polygonMesh);
    }
    else {
        context_.meshBuilder.addPolygon(mesh_, polygon, 
            regionContext.geometryOptions, regionContext.appearanceOptions);
        context_.meshBuilder.writeTextureMappingInfo(mesh_, regionContext.appearanceOptions);

        TerraExtras::Context extrasContext(mesh_, regionContext.style);
        addExtrasIfNecessary(mesh_, extrasContext, regionContext);
    }
}

void SurfaceGenerator::addExtrasIfNecessary(Mesh& mesh,
                                            TerraExtras::Context& extrasContext,
                                            const RegionContext& regionContext) const
{
    auto meshExtras = regionContext.style.getString(regionContext.prefix + StyleConsts::MeshExtrasKey);
    if (meshExtras.empty())
        return;

    ExtrasFuncs.at(meshExtras)(context_, extrasContext);
}
