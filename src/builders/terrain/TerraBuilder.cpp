#include "BoundingBox.hpp"
#include "Exceptions.hpp"
#include "clipper/clipper.hpp"
#include "builders/BuilderContext.hpp"
#include "builders/terrain/LineGridSplitter.hpp"
#include "builders/terrain/TerraBuilder.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "entities/ElementVisitor.hpp"
#include "meshing/Polygon.hpp"
#include "meshing/MeshBuilder.hpp"
#include "utils/CompatibilityUtils.hpp"
#include "utils/GeoUtils.hpp"
#include "utils/MapCssUtils.hpp"
#include "utils/NoiseUtils.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iterator>
#include <sstream>
#include <map>
#include <unordered_map>
#include <vector>

using namespace ClipperLib;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::heightmap;
using namespace utymap::mapcss;
using namespace utymap::meshing;
using namespace utymap::utils;

const uint64_t Scale = 1E7;         // max precision for Lat/Lon: seven decimal positions
const double Tolerance = 10;        // Tolerance for splitting algorithm
const double AreaTolerance = 100;   // Tolerance for meshing

// Represents terrain region points.
struct Region
{
    bool isLayer;
    std::shared_ptr<MeshBuilder::Options> options; // optional: might be empty if polygon is layer
    Paths points;
};

typedef std::vector<Point> Points;
typedef std::vector<Region> Regions;
typedef std::unordered_map<std::string, Regions> Layers;

// mapcss specific keys
const static std::string TerrainLayerKey = "terrain-layer";
const static std::string ColorNoiseFreqKey = "color-noise-freq";
const static std::string EleNoiseFreqKey = "ele-noise-freq";
const static std::string GradientKey= "color";
const static std::string MaxAreaKey = "max-area";
const static std::string WidthKey = "width";
const static std::string HeightKey = "height";
const static std::string LayerPriorityKey = "layer-priority";
const static std::string MeshNameKey = "mesh-name";

class TerraBuilder::TerraBuilderImpl : public ElementBuilder
{
public:

    TerraBuilderImpl(const BuilderContext& context) :
        ElementBuilder(context), splitter_(), mesh_("terrain")
    {
    }

    void visitNode(const utymap::entities::Node& node)
    {
    }

    void visitWay(const utymap::entities::Way& way)
    {
        Style style = context_.styleProvider.forElement(way, context_.quadKey.levelOfDetail);
        Region region = createRegion(style, way.coordinates);

        // make polygon from line by offsetting it using width specified
        double width = utymap::utils::getDouble(WidthKey, context_.stringTable, style);
        Paths offsetSolution;
        offset_.AddPaths(region.points, jtMiter, etOpenSquare);
        offset_.Execute(offsetSolution, width * Scale);
        offset_.Clear();
        region.points = offsetSolution;

        std::string type = region.isLayer ? utymap::utils::getString(TerrainLayerKey, context_.stringTable, style) : "";
        layers_[type].push_back(region);
    }

    void visitArea(const utymap::entities::Area& area)
    {
        Style style = context_.styleProvider.forElement(area, context_.quadKey.levelOfDetail);
        Region region = createRegion(style, area.coordinates);
        std::string type = region.isLayer
            ? utymap::utils::getString(TerrainLayerKey, context_.stringTable, style)
            : "";
        layers_[type].push_back(region);
    }

    void visitRelation(const utymap::entities::Relation& relation)
    {
        Region region;
        struct RelationVisitor : public ElementVisitor
        {
            const Relation& relation;
            TerraBuilder::TerraBuilderImpl& builder;
            Region& region;

            RelationVisitor(TerraBuilder::TerraBuilderImpl& builder, const Relation& relation, Region& region) :
                builder(builder), relation(relation), region(region) {}

            void visitNode(const utymap::entities::Node& node) { node.accept(builder); }

            void visitWay(const utymap::entities::Way& way) { way.accept(builder); }

            void visitArea(const utymap::entities::Area& area)
            {
                Path path;
                path.reserve(area.coordinates.size());
                for (const GeoCoordinate& c : area.coordinates)
                    path.push_back(IntPoint(c.longitude * Scale, c.latitude * Scale));
                
                region.points.push_back(path);
            }

            void visitRelation(const utymap::entities::Relation& relation)  { relation.accept(builder); }

        } visitor(*this, relation, region);

        for (const auto& element : relation.elements) {
            // if there are no tags, then this element is result of clipping
            if (element->tags.empty())
                element->tags = relation.tags;
            element->accept(visitor);
        }

        if (!region.points.empty()) {
            Style style = context_.styleProvider.forElement(relation, context_.quadKey.levelOfDetail);
            region.isLayer = style.has(context_.stringTable.getId(TerrainLayerKey));
            if (!region.isLayer)
                region.options = createMeshOptions(style, "");

            std::string type = region.isLayer 
                ? utymap::utils::getString(TerrainLayerKey, context_.stringTable, style) 
                : "";
            layers_[type].push_back(region);
        }
    }

    // builds tile mesh using data provided.
    void complete()
    {
        Style style = context_.styleProvider.forCanvas(context_.quadKey.levelOfDetail);
        double step = utymap::utils::getDouble(MaxAreaKey, context_.stringTable, style);
        splitter_.setParams(Scale, step, Tolerance);

        BoundingBox bbox = utymap::utils::GeoUtils::quadKeyToBoundingBox(context_.quadKey);
        rect_ = Rectangle(bbox.minPoint.longitude, bbox.minPoint.latitude,
            bbox.maxPoint.longitude, bbox.maxPoint.latitude);

        buildLayers(style);
        buildBackground(style);

        context_.meshCallback(mesh_);
    }

private:

    // process all found layers.
    void buildLayers(const Style& style)
    {
        // 1. process layers: regions with shared properties.
        std::stringstream ss(utymap::utils::getString(LayerPriorityKey, context_.stringTable, style));
        while (ss.good()) {
            std::string name;
            getline(ss, name, ',');
            auto layer = layers_.find(name);
            if (layer != layers_.end()) {
                buildFromRegions(layer->second, createMeshOptions(style, name + "-"));
                layers_.erase(layer);
            }
        }

        // 2. Process the rest: each region has aready its own properties.
        for (auto& layer : layers_)
            for (auto& region : layer.second) {
                buildFromPaths(region.points, region.options, false);
            }
    }

    // process the rest area.
    void buildBackground(const Style& style)
    {
        BoundingBox bbox = utymap::utils::GeoUtils::quadKeyToBoundingBox(context_.quadKey);
        Path tileRect;
        tileRect.push_back(IntPoint(bbox.minPoint.longitude*Scale, bbox.minPoint.latitude *Scale));
        tileRect.push_back(IntPoint(bbox.maxPoint.longitude *Scale, bbox.minPoint.latitude *Scale));
        tileRect.push_back(IntPoint(bbox.maxPoint.longitude *Scale, bbox.maxPoint.latitude*Scale));
        tileRect.push_back(IntPoint(bbox.minPoint.longitude*Scale, bbox.maxPoint.latitude*Scale));

        clipper_.AddPath(tileRect, ptSubject, true);
        clipper_.AddPaths(backgroundClipArea_, ptClip, true);
        Paths background;
        clipper_.Execute(ctDifference, background, pftPositive, pftPositive);
        clipper_.Clear();

        if (!background.empty())
            populateMesh(createMeshOptions(style, ""), background);
    }

    Region createRegion(const Style& style, const std::vector<GeoCoordinate>& coordinates)
    {
        Region region;
        Path path;
        path.reserve(coordinates.size());
        for (const GeoCoordinate& c : coordinates) 
            path.push_back(IntPoint(c.longitude * Scale, c.latitude * Scale));

        region.points.push_back(path);

        region.isLayer = style.has(context_.stringTable.getId(TerrainLayerKey));
        if (!region.isLayer)
            region.options = createMeshOptions(style, "");

        return std::move(region);
    }

    std::shared_ptr<MeshBuilder::Options> createMeshOptions(const Style& style, const std::string& prefix)
    {
        std::string gradientKey = utymap::utils::getString(prefix + GradientKey, context_.stringTable, style);
        return std::shared_ptr<MeshBuilder::Options>(new MeshBuilder::Options(
               utymap::utils::getDouble(prefix + MaxAreaKey, context_.stringTable, style),
               utymap::utils::getDouble(prefix + EleNoiseFreqKey, context_.stringTable, style),
               utymap::utils::getDouble(prefix + ColorNoiseFreqKey, context_.stringTable, style),
               utymap::utils::getDouble(prefix + HeightKey, context_.stringTable, style, 0),
               context_.styleProvider.getGradient(gradientKey),
               utymap::utils::getString(prefix + MeshNameKey, context_.stringTable, style, "")));
    }

    void buildFromRegions(const Regions& regions, const std::shared_ptr<MeshBuilder::Options>& options)
    {
        // merge all regions together
        Clipper clipper;
        for (const Region& region : regions)
            clipper.AddPaths(region.points, ptSubject, true);

        Paths result;
        clipper.Execute(ctUnion, result, pftPositive, pftPositive);

        buildFromPaths(result, options);
    }

    void buildFromPaths(Paths& paths, const std::shared_ptr<MeshBuilder::Options>& options, bool moveSubjectToClip = true)
    {
        clipper_.AddPaths(paths, ptSubject, true);
        paths.clear();
        clipper_.Execute(ctDifference, paths, pftPositive, pftPositive);
        // NOTE: this is performance optimization: we cannot make all
        // polygons to be clipping as it slows down clipper dramatically.
        if (moveSubjectToClip) {
            clipper_.moveSubjectToClip();
        } 
        else {
            backgroundClipArea_.insert(backgroundClipArea_.end(), paths.begin(), paths.end());
            clipper_.removeSubject();
        }
        populateMesh(options, paths);
    }

    void populateMesh(const std::shared_ptr<MeshBuilder::Options>& options, Paths& paths)
    {
        ClipperLib::SimplifyPolygons(paths);
        ClipperLib::CleanPolygons(paths);

        bool hasHeightOffset = std::abs(options->heightOffset) > 1E-8;
        // calculate approximate size of overall points
        auto size = 0;
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
                processHeightOffset(options, points, isHole);
        }

        if (!polygon.points.empty())
            fillMesh(options, polygon);
    }

    // restores mesh points from clipper points and injects new ones according to grid.
    Points restorePoints(const Path& path)
    {
        int lastItemIndex = path.size() - 1;
        Points points;
        points.reserve(path.size());
        for (int i = 0; i <= lastItemIndex; i++)
            splitter_.split(path[i], path[i == lastItemIndex ? 0 : i + 1], points);

        return std::move(points);
    }

    void fillMesh(const std::shared_ptr<MeshBuilder::Options>& options, Polygon& polygon)
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

    void processHeightOffset(const std::shared_ptr<MeshBuilder::Options>& options, const Points& points, bool isHole)
    {
        auto index = mesh_.vertices.size() / 3;
        for (auto i = 0; i < points.size(); ++i) {
            Point p1 = points[i];
            Point p2 = points[i == (points.size() - 1) ? 0 : i + 1];

            // check whether two points are on cell rect
            if (rect_.isOnBorder(p1) && rect_.isOnBorder(p2)) continue;

            context_.meshBuilder.addPlane(mesh_, p1, p2, *options);
        }
    }

ClipperEx clipper_;
ClipperOffset offset_;
LineGridSplitter splitter_;
Rectangle rect_;
Layers layers_;
Paths backgroundClipArea_;
Mesh mesh_;
};

void TerraBuilder::visitNode(const utymap::entities::Node& node) { pimpl_->visitNode(node); }

void TerraBuilder::visitWay(const utymap::entities::Way& way) { pimpl_->visitWay(way); }

void TerraBuilder::visitArea(const utymap::entities::Area& area) { pimpl_->visitArea(area); }

void TerraBuilder::visitRelation(const utymap::entities::Relation& relation) { pimpl_->visitRelation(relation); }

void TerraBuilder::complete() { pimpl_->complete(); }

TerraBuilder::~TerraBuilder() { }

TerraBuilder::TerraBuilder(const BuilderContext& context) :
    utymap::builders::ElementBuilder(context),
    pimpl_(new TerraBuilder::TerraBuilderImpl(context))
{
}
