#include "BoundingBox.hpp"
#include "Exceptions.hpp"
#include "clipper/clipper.hpp"
#include "builders/TerraBuilder.hpp"
#include "entities/ElementVisitor.hpp"
#include "meshing/Polygon.hpp"
#include "meshing/MeshBuilder.hpp"
#include "meshing/LineGridSplitter.hpp"
#include "index/GeoUtils.hpp"
#include "utils/CompatibilityUtils.hpp"
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

const uint64_t Scale = 1E7; // max precision for Lat/Lon: seven decimal positions
const double Tolerance = 10; // Tolerance for splitting algorithm
const double AreaTolerance = 100; // Tolerance for meshing

// Properties of terrain region union.
struct Properties
{
    std::string gradientKey;
    double eleNoiseFreq;
    double colorNoiseFreq;
    double heightOffset;
    double maxArea;
    std::string meshName;

    Properties() : gradientKey(), eleNoiseFreq(0), colorNoiseFreq(0),
        heightOffset(0), maxArea(0), meshName()
    {
    }
};

// Represents terrain region points.
struct Region
{
    bool isLayer;
    Properties properties;
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

class TerraBuilder::TerraBuilderImpl : public ElementVisitor
{
public:

    TerraBuilderImpl(const QuadKey& quadKey, const StyleProvider& styleProvider, StringTable& stringTable, 
                     ElevationProvider& eleProvider, std::function<void(const Mesh&)> callback) :
         quadKey_(quadKey), styleProvider_(styleProvider), stringTable_(stringTable), 
         eleProvider_(eleProvider), meshBuilder_(eleProvider), callback_(callback), splitter_()
    {
    }

    void visitNode(const utymap::entities::Node& node)
    {
    }

    void visitWay(const utymap::entities::Way& way)
    {
        Style style = styleProvider_.forElement(way, quadKey_.levelOfDetail);
        Region region = createRegion(style, way.coordinates);

        // make polygon from line by offsetting it using width specified
        double width = utymap::utils::getDouble(WidthKey, stringTable_, style);
        Paths offsetSolution;
        offset_.AddPaths(region.points, jtMiter, etOpenSquare);
        offset_.Execute(offsetSolution, width * Scale);
        offset_.Clear();
        region.points = offsetSolution;

        std::string type = region.isLayer ? utymap::utils::getString(TerrainLayerKey, stringTable_, style) : "";
        layers_[type].push_back(region);
    }

    void visitArea(const utymap::entities::Area& area)
    {
        Style style = styleProvider_.forElement(area, quadKey_.levelOfDetail);
        Region region = createRegion(style, area.coordinates);
        std::string type = region.isLayer 
            ? utymap::utils::getString(TerrainLayerKey, stringTable_, style)
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
                for (const GeoCoordinate& c : area.coordinates) {
                    path.push_back(IntPoint(c.longitude * Scale, c.latitude * Scale));
                }
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
            Style style = styleProvider_.forElement(relation, quadKey_.levelOfDetail);
            region.isLayer = style.has(stringTable_.getId(TerrainLayerKey));
            if (!region.isLayer) {
                region.properties = createRegionProperties(style, "");
            }
            std::string type = region.isLayer ? utymap::utils::getString(TerrainLayerKey, stringTable_, style) : "";
            layers_[type].push_back(region);
        }
    }

    // builds tile mesh using data provided.
    void build()
    {
         configureSplitter(quadKey_.levelOfDetail);

        Style style = styleProvider_.forCanvas(quadKey_.levelOfDetail);
        BoundingBox bbox = utymap::index::GeoUtils::quadKeyToBoundingBox(quadKey_);
        rect_ = Rectangle(bbox.minPoint.longitude, bbox.minPoint.latitude, 
            bbox.maxPoint.longitude, bbox.maxPoint.latitude);

        buildLayers(style);
        buildBackground(style);
        
        mesh_.name = "terrain";
        callback_(mesh_);
    }
private:

    void configureSplitter(int levelOfDetails)
    {    
        // TODO
        switch (levelOfDetails)
        {
            case 1: splitter_.setParams(Scale, 3, Tolerance); break;
            default: throw std::domain_error("Unknown Level of details:" + std::to_string(levelOfDetails));
        };
    }

    // process all found layers.
    void buildLayers(const Style& style)
    {
        // 1. process layers: regions with shared properties.
        std::stringstream ss(utymap::utils::getString(LayerPriorityKey, stringTable_, style));
        while (ss.good())
        {
            std::string name;
            getline(ss, name, ',');
            auto layer = layers_.find(name);
            if (layer != layers_.end()) {
                Properties properties = createRegionProperties(style, name + "-");
                buildFromRegions(layer->second, properties);
                layers_.erase(layer);
            }
        }

        // 2. Process the rest: each region has aready its own properties.
        for (auto& layer : layers_) {
            for (auto& region : layer.second) {
                buildFromPaths(region.points, region.properties, false);
            }
        }
    }

    // process the rest area.
    void buildBackground(const Style& style)
    {
        BoundingBox bbox = utymap::index::GeoUtils::quadKeyToBoundingBox(quadKey_);
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
            populateMesh(createRegionProperties(style, ""), background);
    }

    Region createRegion(const Style& style, const std::vector<GeoCoordinate>& coordinates)
    {
        Region region;
        Path path;
        path.reserve(coordinates.size());
        for (const GeoCoordinate& c : coordinates) {
            path.push_back(IntPoint(c.longitude * Scale, c.latitude * Scale));
        }
        region.points.push_back(path);

        region.isLayer = style.has(stringTable_.getId(TerrainLayerKey));
        if (!region.isLayer)
            region.properties = createRegionProperties(style, "");

        return std::move(region);
    }

    Properties createRegionProperties(const Style& style, const std::string& prefix)
    {
        Properties properties;
        // required
        properties.eleNoiseFreq = utymap::utils::getDouble(prefix + EleNoiseFreqKey, stringTable_, style);
        properties.colorNoiseFreq = utymap::utils::getDouble(prefix + ColorNoiseFreqKey, stringTable_, style);
        properties.gradientKey = utymap::utils::getString(prefix + GradientKey, stringTable_, style);
        properties.maxArea = utymap::utils::getDouble(prefix + MaxAreaKey, stringTable_, style);
        // optional
        properties.heightOffset = utymap::utils::getDouble(prefix + HeightKey, stringTable_, style, 0);
        properties.meshName = utymap::utils::getString(prefix + MeshNameKey, stringTable_, style, "");

        return std::move(properties);
    }

    void buildFromRegions(const Regions& regions, const Properties& properties)
    {
        // merge all regions together
        Clipper clipper;
        for (const Region& region : regions) {
            clipper.AddPaths(region.points, ptSubject, true);
        }
        Paths result;
        clipper.Execute(ctUnion, result, pftPositive, pftPositive);

        buildFromPaths(result, properties);
    }

    void buildFromPaths(Paths& paths, const Properties& properties, bool moveSubjectToClip = true)
    {
        clipper_.AddPaths(paths, ptSubject, true);
        paths.clear();
        clipper_.Execute(ctDifference, paths, pftPositive, pftPositive);
        // NOTE: this is performance optimization: we cannot make all 
        // polygons to be clipping as it slows down clipper dramatically.
        if (moveSubjectToClip)
            clipper_.moveSubjectToClip();
        else {
            backgroundClipArea_.insert(backgroundClipArea_.end(), paths.begin(), paths.end());
            clipper_.removeSubject();
        }
        populateMesh(properties, paths);
    }
   
    void populateMesh(const Properties& properties, Paths& paths)
    {
        ClipperLib::SimplifyPolygons(paths);
        ClipperLib::CleanPolygons(paths);

        bool hasHeightOffset = std::abs(properties.heightOffset) > 1E-8;
        // calculate approximate size of overall points
        auto size = 0;
        for (auto i = 0; i < paths.size(); ++i) {
            size += paths[i].size() * 1.5;
        }

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
                processHeightOffset(properties, points, isHole);
        }

        if (!polygon.points.empty()) {
            fillMesh(properties, polygon);
        }
    }

    // restores mesh points from clipper points and injects new ones according to grid.
    Points restorePoints(const Path& path)
    {
        int lastItemIndex = path.size() - 1;
        Points points;
        points.reserve(path.size());
        for (int i = 0; i <= lastItemIndex; i++) {
            splitter_.split(path[i], path[i == lastItemIndex ? 0 : i + 1], points);
        }

        return std::move(points);
    }

    void fillMesh(const Properties& properties, Polygon& polygon)
    {
        Mesh polygonMesh = meshBuilder_.build(polygon, MeshBuilder::Options
        {
            properties.maxArea,
            properties.eleNoiseFreq,
            properties.colorNoiseFreq,
            properties.heightOffset,
            styleProvider_.getGradient(properties.gradientKey),
            /* segmentSplit=*/ 0
        });

        if (properties.meshName != "") {
            polygonMesh.name = properties.meshName;
            callback_(polygonMesh);
        }
        else {
            auto startVertIndex = mesh_.vertices.size() / 3;
            mesh_.vertices.insert(mesh_.vertices.end(),
                polygonMesh.vertices.begin(),
                polygonMesh.vertices.end());

            for (const auto& tri : polygonMesh.triangles) {
                mesh_.triangles.push_back(startVertIndex + tri);
            }

            mesh_.colors.insert(mesh_.colors.end(),
                polygonMesh.colors.begin(),
                polygonMesh.colors.end());
        }
    }

    void processHeightOffset(const Properties& properties, const Points& points, bool isHole)
    {
        ColorGradient gradient = styleProvider_.getGradient(properties.gradientKey);
        auto index = mesh_.vertices.size() / 3;
        for (auto i = 0; i < points.size(); ++i) {
            Point p1 = points[i];
            Point p2 = points[i == (points.size() - 1) ? 0 : i + 1];

            // check whether two points are on cell rect
            if (rect_.isOnBorder(p1) && rect_.isOnBorder(p2))
                continue;

            double ele1 = eleProvider_.getElevation(p1.x, p1.y);
            double ele2 = eleProvider_.getElevation(p2.x, p2.y);

            ele1 += NoiseUtils::perlin3D(p1.x, ele1, p1.y, properties.eleNoiseFreq);
            ele2 += NoiseUtils::perlin3D(p2.x, ele2, p2.y, properties.eleNoiseFreq);

            Color color1 = gradient.evaluate((NoiseUtils::perlin3D(p1.x, ele1, p1.y, properties.colorNoiseFreq) + 1) / 2);
            Color color2 = gradient.evaluate((NoiseUtils::perlin3D(p2.x, ele2, p2.y, properties.colorNoiseFreq) + 1) / 2);

            addVertex(p1, ele1, color1, index);
            addVertex(p2, ele2, color1, index + 2);
            addVertex(p2, ele2 + properties.heightOffset, color1, index + 1);
            index += 3;

            addVertex(p1, ele1 + properties.heightOffset, color1, index);
            addVertex(p1, ele1, color1, index + 2);
            addVertex(p2, ele2 + properties.heightOffset, color1, index + 1);
            index += 3;
        }
    }

    inline void addVertex(const Point& p, double ele, const Color& color, int index)
    {
        mesh_.vertices.push_back(p.x);
        mesh_.vertices.push_back(p.y);
        mesh_.vertices.push_back(ele);
        mesh_.colors.push_back(color);
        mesh_.triangles.push_back(index);
    }

const StyleProvider& styleProvider_;
ElevationProvider& eleProvider_;
StringTable& stringTable_;
std::function<void(const Mesh&)> callback_;

ClipperEx clipper_;
ClipperOffset offset_;
LineGridSplitter splitter_;
MeshBuilder meshBuilder_;
QuadKey quadKey_;
Rectangle rect_;
Layers layers_;
Paths backgroundClipArea_;
Mesh mesh_;
};

void TerraBuilder::visitNode(const utymap::entities::Node& node) { pimpl_->visitNode(node); }

void TerraBuilder::visitWay(const utymap::entities::Way& way) { pimpl_->visitWay(way); }

void TerraBuilder::visitArea(const utymap::entities::Area& area) { pimpl_->visitArea(area); }

void TerraBuilder::visitRelation(const utymap::entities::Relation& relation) { pimpl_->visitRelation(relation); }

void TerraBuilder::complete() { pimpl_->build(); }

TerraBuilder::~TerraBuilder() { }

TerraBuilder::TerraBuilder(const utymap::QuadKey& quadKey,
                           const utymap::mapcss::StyleProvider& styleProvider,
                           utymap::index::StringTable& stringTable,
                           utymap::heightmap::ElevationProvider& eleProvider,
                           std::function<void(const utymap::meshing::Mesh&)> callback) :
    pimpl_(std::unique_ptr<TerraBuilder::TerraBuilderImpl>(new TerraBuilder::TerraBuilderImpl(quadKey, styleProvider, stringTable, eleProvider, callback)))
{
}
