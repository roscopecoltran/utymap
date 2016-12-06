#include "builders/terrain/TerraGenerator.hpp"

using namespace ClipperLib;
using namespace utymap::builders;
using namespace utymap::mapcss;
using namespace utymap::math;

namespace {
    const double Scale = 1E7;
}

TerraGenerator::TerraGenerator(const utymap::builders::BuilderContext& context,
                               const utymap::mapcss::Style& style,
                               const ClipperLib::Path& tileRect) :
                               context_(context), style_(style), tileRect_(tileRect),
                               rect_(context.boundingBox.minPoint.longitude,
                                     context.boundingBox.minPoint.latitude,
                                     context.boundingBox.maxPoint.longitude,
                                     context.boundingBox.maxPoint.latitude)
{
    auto size = style_.getValue(StyleConsts::GridCellSize, context_.boundingBox.height(), context_.boundingBox.center());
    splitter_.setParams(Scale, size);
}

void TerraGenerator::addRegion(const std::string& type, const utymap::entities::Element& element, const Style& style, std::shared_ptr<Region> region)
{
    layers_[type].push(region);
}

void TerraGenerator::buildForeground()
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

void TerraGenerator::buildBackground()
{
    Paths background;
    backgroundClipper_.AddPath(tileRect_, ptSubject, true);
    backgroundClipper_.Execute(ctDifference, background, pftNonZero, pftNonZero);
    backgroundClipper_.Clear();

    if (!background.empty())
        addGeometry(background, RegionContext::create(context_, style_, ""));
}

void TerraGenerator::buildFromRegions(Regions& regions, const RegionContext& regionContext)
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

void TerraGenerator::buildFromPaths(const Paths& paths, const RegionContext& regionContext)
{
    Paths solution;
    foregroundClipper_.AddPaths(paths, ptSubject, true);
    foregroundClipper_.Execute(ctDifference, solution, pftNonZero, pftNonZero);
    foregroundClipper_.moveSubjectToClip();

    ClipperLib::SimplifyPolygons(solution);
    ClipperLib::CleanPolygons(solution);

    addGeometry(solution, regionContext);
}

std::vector<Vector2> TerraGenerator::restoreGeometry(const Path& geometry) const
{
    auto lastItemIndex = geometry.size() - 1;
    std::vector<utymap::math::Vector2> points;
    points.reserve(geometry.size());
    for (int i = 0; i <= lastItemIndex; i++)
        splitter_.split(geometry[i], geometry[i == lastItemIndex ? 0 : i + 1], points);

    return std::move(points);
}