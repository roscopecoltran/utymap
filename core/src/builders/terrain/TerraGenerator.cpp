#include "builders/terrain/TerraGenerator.hpp"
#include "utils/GeoUtils.hpp"
#include "utils/MapCssUtils.hpp"

#include <iterator>

using namespace ClipperLib;
using namespace utymap::builders;
using namespace utymap::mapcss;
using namespace utymap::meshing;
using namespace utymap::utils;

namespace {
    const double AreaTolerance = 1000; // Tolerance for meshing
    const static double Scale = 1E7;

    const static std::string TerrainMeshName = "terrain";
    const static std::string ColorNoiseFreqKey = "color-noise-freq";
    const static std::string EleNoiseFreqKey = "ele-noise-freq";
    const static std::string GradientKey = "color";
    const static std::string MaxAreaKey = "max-area";
    const static std::string HeightKey = "height";
    const static std::string LayerPriorityKey = "layer-priority";
    const static std::string MeshNameKey = "mesh-name";
    const static std::string GridCellSize = "grid-cell-size";
};

TerraGenerator::TerraGenerator(const BuilderContext& context, const utymap::BoundingBox& bbox, 
                               const Style& style, ClipperEx& clipper) :
        context_(context), mesh_(TerrainMeshName), bbox_(bbox), style_(style), clipper_(clipper),
        rect_(bbox_.minPoint.longitude, bbox_.minPoint.latitude, bbox_.maxPoint.longitude, bbox_.maxPoint.latitude)
{
}

void TerraGenerator::addRegion(const std::string& type, const Region& region)
{
    layers_[type].push_back(region);
}

void TerraGenerator::generate(Path& tileRect)
{
    double size = utymap::utils::getDimension(GridCellSize, context_.stringTable, style_,
                                              bbox_.maxPoint.latitude - bbox_.minPoint.latitude, bbox_.center());
    splitter_.setParams(Scale, size);

    buildLayers();
    buildBackground(tileRect);

    context_.meshCallback(mesh_);
}

// process all found layers.
void TerraGenerator::buildLayers()
{
    // 1. process layers: regions with shared properties.
    std::stringstream ss(*utymap::utils::getString(LayerPriorityKey, context_.stringTable, style_));
    while (ss.good()) {
        std::string name;
        getline(ss, name, ',');
        auto layer = layers_.find(name);
        if (layer != layers_.end()) {
            buildFromRegions(layer->second, createMeshOptions(style_, name + "-"));
            layers_.erase(layer);
        }
    }

    // 2. Process the rest: each region has aready its own properties.
    for (auto& layer : layers_)
        for (auto& region : layer.second) {
        buildFromPaths(region.points, region.options);
        }
}

// process the rest area.
void TerraGenerator::buildBackground(Path& tileRect)
{
    clipper_.AddPath(tileRect, ptSubject, true);
    Paths background;
    clipper_.Execute(ctDifference, background, pftPositive, pftPositive);
    clipper_.Clear();

    if (!background.empty())
        populateMesh(background, createMeshOptions(style_, ""));
}

std::shared_ptr<MeshBuilder::Options> TerraGenerator::createMeshOptions(const Style& style, const std::string& prefix)
{
    double quadKeyWidth = bbox_.maxPoint.latitude - bbox_.minPoint.latitude;
    auto gradientKey = utymap::utils::getString(prefix + GradientKey, context_.stringTable, style);
    return std::shared_ptr<MeshBuilder::Options>(new MeshBuilder::Options(
        utymap::utils::getDimension(prefix + MaxAreaKey, context_.stringTable, style, quadKeyWidth * quadKeyWidth),
        utymap::utils::getDouble(prefix + EleNoiseFreqKey, context_.stringTable, style),
        utymap::utils::getDouble(prefix + ColorNoiseFreqKey, context_.stringTable, style),
        utymap::utils::getDimension(prefix + HeightKey, context_.stringTable, style, quadKeyWidth, 0),
        context_.styleProvider.getGradient(*gradientKey),
        std::numeric_limits<double>::lowest(),
        *utymap::utils::getString(prefix + MeshNameKey, context_.stringTable, style, ""),
        /* no new vertices on boundaries */ 1));
}

void TerraGenerator::buildFromRegions(const Regions& regions, const std::shared_ptr<MeshBuilder::Options>& options)
{
    // merge all regions together
    Clipper clipper;
    for (const Region& region : regions)
        clipper.AddPaths(region.points, ptSubject, true);

    Paths result;
    clipper.Execute(ctUnion, result, pftPositive, pftPositive);

    buildFromPaths(result, options);
}

void TerraGenerator::buildFromPaths(Paths& paths, const std::shared_ptr<MeshBuilder::Options>& options)
{
    clipper_.AddPaths(paths, ptSubject, true);
    paths.clear();
    clipper_.Execute(ctDifference, paths, pftPositive, pftPositive);
    clipper_.moveSubjectToClip();

    populateMesh(paths, options);
}

void TerraGenerator::populateMesh(Paths& paths, const std::shared_ptr<MeshBuilder::Options>& options)
{
    ClipperLib::SimplifyPolygons(paths);
    ClipperLib::CleanPolygons(paths);

    bool hasHeightOffset = std::abs(options->heightOffset) > 1E-8;
    // calculate approximate size of overall points
    size_t size = 0;
    for (auto i = 0; i < paths.size(); ++i)
        size += paths[i].size() * 1.5;

    Polygon polygon(size);
    for (const Path& path : paths) {
        double area = ClipperLib::Area(path);
        bool isHole = area < 0;
        if (std::abs(area) < AreaTolerance)
            continue;

        Points points = restorePoints(path);
        if (isHole)
            polygon.addHole(points);
        else
            polygon.addContour(points);

        if (hasHeightOffset)
            processHeightOffset(points, options);
    }

    if (!polygon.points.empty())
        fillMesh(polygon, options);
}

// restores mesh points from clipper points and injects new ones according to grid.
TerraGenerator::Points TerraGenerator::restorePoints(const Path& path)
{
    auto lastItemIndex = path.size() - 1;
    Points points;
    points.reserve(path.size());
    for (int i = 0; i <= lastItemIndex; i++)
        splitter_.split(path[i], path[i == lastItemIndex ? 0 : i + 1], points);

    return std::move(points);
}

void TerraGenerator::fillMesh(Polygon& polygon, const std::shared_ptr<MeshBuilder::Options>& options)
{
    if (options->meshName != "") {
        Mesh polygonMesh(options->meshName);
        context_.meshBuilder.addPolygon(polygonMesh, polygon, *options);
        context_.meshCallback(polygonMesh);
    }
    else {
        context_.meshBuilder.addPolygon(mesh_, polygon, *options);
    }
}

void TerraGenerator::processHeightOffset(const Points& points, const std::shared_ptr<MeshBuilder::Options>& options)
{
    // do not use elevation noise for height offset.
    auto newOptions = *options;
    newOptions.eleNoiseFreq = 0;

    for (auto i = 0; i < points.size(); ++i) {
        Point p1 = points[i];
        Point p2 = points[i == (points.size() - 1) ? 0 : i + 1];

        // check whether two points are on cell rect
        if (rect_.isOnBorder(p1) && rect_.isOnBorder(p2))
            continue;

        context_.meshBuilder.addPlane(mesh_, p1, p2, newOptions);
    }
}