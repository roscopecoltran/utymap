#include "builders/terrain/SurfaceGenerator.hpp"

using namespace ClipperLib;
using namespace utymap::builders;
using namespace utymap::mapcss;
using namespace utymap::math;
using namespace utymap::utils;

using namespace std::placeholders;

namespace {
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
    // Process layers according their priority
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

    // Process the rest.
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
        TerraGenerator::addGeometry(background,
                                    RegionContext::create(context_, style_, ""),
                                    [](const Path& path) {});
}

void SurfaceGenerator::buildLayer(Layer& layer)
{
    for (const auto& region : layer) {
        if (region->level == 0)
            buildFromPaths(region->geometry, *region->context);
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

    TerraGenerator::addGeometry(solution, regionContext, [&](const Path& path) {
        backgroundClipper_.AddPath(path, ptClip, true);
    });
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
