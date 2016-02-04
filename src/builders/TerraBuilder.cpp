#include "BoundingBox.hpp"
#include "Exceptions.hpp"
#include "clipper/clipper.hpp"
#include "builders/TerraBuilder.hpp"
#include "entities/ElementVisitor.hpp"
#include "meshing/Polygon.hpp"
#include "meshing/MeshBuilder.hpp"
#include "meshing/LineGridSplitter.hpp"
#include "index/GeoUtils.hpp"
#include "utils/MapCssUtils.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iterator>
#include <map>
#include <unordered_map>
#include <vector>

using namespace ClipperLib;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::heightmap;
using namespace utymap::mapcss;
using namespace utymap::meshing;

const uint64_t Scale = 1E7; // max precision for Lat/Lon: seven decimal positions
const double Tolerance = 10; // Tolerance for splitting algorithm
const double AreaTolerance = 100; // Tolerance for meshing

// Represents terrain region.
struct MeshRegion
{
    struct Properties
    {
        std::string gradientKey;
        std::string textureAtlas;
        std::string textureKey;
        float eleNoiseFreq;
        float colorNoiseFreq;
        float heightOffset;

        Properties() : gradientKey(), textureAtlas(), textureKey(),
            eleNoiseFreq(0), colorNoiseFreq(0), heightOffset(0)
        {
        }
    };

    utymap::meshing::Contour points;
    std::vector<utymap::meshing::Contour> holes;
    Properties properties;
};

typedef std::vector<MeshRegion> MeshRegions;
typedef std::unordered_map<int, MeshRegions> RoadMap;
typedef std::vector<Point> MeshPoints;

class TerraBuilder::TerraBuilderImpl : public ElementVisitor
{
    const std::string TypeKey = "terrain-type";
    const std::string ColorNoiseFreqKey = "color-noise-freq";
    const std::string EleNoiseFreqKey = "ele-noise-freq";
    const std::string GradientKey = "color";
    const std::string WaterKey = "water";
    const std::string SurfaceKey = "surface";

public:

    TerraBuilderImpl(utymap::index::StringTable& stringTable,
                     const utymap::mapcss::StyleProvider& styleProvider,
                     ElevationProvider& eleProvider,
                     std::function<void(const Mesh&)> callback) :
        stringTable_(stringTable),
        styleProvider_(styleProvider),
        meshBuilder_(eleProvider),
        callback_(callback),
        quadKey_(),
        splitter_()
    {
    }

    inline void setQuadKey(const QuadKey& quadKey)
    {
        quadKey_ = quadKey;
    }

    void visitNode(const utymap::entities::Node& node)
    {
    }

    void visitWay(const utymap::entities::Way& way)
    {
        //carRoads_[width].push_back(carRoad);
        // walkRoads_[width].push_back(walkRoad);
    }

    void visitArea(const utymap::entities::Area& area)
    {
        Style style = styleProvider_.forElement(area, quadKey_.levelOfDetail);
        MeshRegion region = createMeshRegion(style, area.coordinates);
        std::string type = utymap::utils::getString(TypeKey, stringTable_, style);

        if (type == SurfaceKey) {
            surfaces_.push_back(region);
        } else if (type == WaterKey) {
            waters_.push_back(region);
        }
        else {
            throw utymap::MapCssException(std::string("Unknown terrain type: ") + type);
        }
    }

    void visitRelation(const utymap::entities::Relation& relation)
    {
        for (const auto& element : relation.elements) {
            // if there are no tags, then this element is result of clipping
            if (element->tags.size() == 0) 
                element->tags = relation.tags;
            element->accept(*this);
        }
    }

    // builds tile mesh using data provided.
    void build()
    {
        configureSplitter(quadKey_.levelOfDetail);

        // fill context with layer specific data.
        buildWater();
        buildRoads();
        buildSurfaces();
        buildBackground();

        callback_(mesh_);
    }
private:

    void configureSplitter(int levelOfDetails)
    {    
        // TODO
        switch (levelOfDetails)
        {
        case 1: splitter_.setParams(Scale, 3, Tolerance); break;
        default: throw std::domain_error("Unknown LoD");
        };
    }

    MeshRegion createMeshRegion(const Style& style, const std::vector<GeoCoordinate>& coordinates)
    {
        MeshRegion region;
        region.properties = createMeshRegionProperties(style);
        region.points.reserve(coordinates.size());
        for (const GeoCoordinate& c : coordinates) {
            region.points.push_back(Point(c.longitude, c.latitude));
        }
        return std::move(region);
    }

    MeshRegion::Properties createMeshRegionProperties(const Style& style)
    {
        MeshRegion::Properties properties;
        properties.eleNoiseFreq = utymap::utils::getFloat(EleNoiseFreqKey, stringTable_, style);
        properties.colorNoiseFreq = utymap::utils::getFloat(ColorNoiseFreqKey, stringTable_, style);
        properties.gradientKey = utymap::utils::getString(GradientKey, stringTable_, style);

        return std::move(properties);
    }

    Paths buildPaths(const MeshRegions& regions) 
    {
        // TODO holes are not processed
        Paths paths;
        paths.reserve(regions.size());
        for (const MeshRegion& region : regions) {
            Path p;
            p.reserve(region.points.size());
            for (const Point point : region.points) {
                p.push_back(IntPoint(point.x*Scale, point.y*Scale));
            }
            paths.push_back(p);
        }
        return std::move(paths);
    }

    Paths buildOffsetSolution(const RoadMap& roads)
    {
        for (auto r : roads) {
            Paths offsetSolution;
            offset_.AddPaths(buildPaths(r.second), jtMiter, etOpenSquare);
            offset_.Execute(offsetSolution, r.first);
            clipper_.AddPaths(offsetSolution, ptSubject, true);
            offset_.Clear();
        }
        Paths polySolution;
        clipper_.Execute(ctUnion, polySolution, pftPositive, pftPositive);
        clipper_.Clear();
        return std::move(polySolution);
    }

    Paths clipRoads(const Paths& roads)
    {
        Paths resultRoads;
        clipper_.AddPaths(waterShape_, ptClip, true);
        clipper_.AddPaths(roads, ptSubject, true);
        clipper_.Execute(ctDifference, resultRoads, pftPositive, pftPositive);
        clipper_.Clear();
        return std::move(resultRoads);
    }

    // populates mesh 
    void populateMesh(const MeshRegion::Properties& properties, Paths& paths)
    {
        bool hasHeightOffset = properties.heightOffset > 0;
        //std::vector<MeshPoints> contours;
        //contours.reserve(hasHeightOffset ? 4 : 0);

        ClipperLib::SimplifyPolygons(paths);

        for (Path path : paths) {
            double area = ClipperLib::Area(path);
            if (std::abs(area) < AreaTolerance) 
                continue;
            
            bool isHole = area < 0;

            MeshPoints points = restorePoints(path);
            Polygon polygon(points.size() * 2, isHole ? 1 : 0);
            if (isHole)
                polygon.addHole(points);
            else
                polygon.addContour(points);
            fillMesh(properties, polygon);

            //if (hasHeightOffset)
            //    contours.push_back(points);
        }

        //if (hasHeightOffset) {
        //    std::reverse(contours.begin(), contours.end());
        //    processHeightOffset(contours);
        //}
    }

    // restores mesh points from clipper points and injects new ones according to grid.
    MeshPoints restorePoints(const Path& path)
    {
        int lastItemIndex = path.size() - 1;
        MeshPoints points;
        points.reserve(path.size());
        for (int i = 0; i <= lastItemIndex; i++) {
            IntPoint start = path[i];
            IntPoint end = path[i == lastItemIndex ? 0 : i + 1];

            splitter_.split(start, end, points);
        }

        return std::move(points);
    }

    void processHeightOffset(const std::vector<MeshPoints>& contours)
    {
        // TODO
    }

    void fillMesh(const MeshRegion::Properties& properties, Polygon& polygon)
    {
        // TODO use valid area value
        Mesh regionMesh = meshBuilder_.build(polygon, MeshBuilder::Options
        {
            /* area=*/ 10,
            /* elevation noise frequency*/ properties.eleNoiseFreq,
            /* color noise frequency. */ properties.colorNoiseFreq,
            styleProvider_.getGradient(properties.gradientKey),
            /* segmentSplit=*/ 0
        });

        auto startVertIndex = mesh_.vertices.size() / 3;

        mesh_.vertices.insert(mesh_.vertices.end(),
            regionMesh.vertices.begin(),
            regionMesh.vertices.end());

        for (const auto& tri : regionMesh.triangles) {
            mesh_.triangles.push_back(startVertIndex + tri);
        }

        mesh_.colors.insert(mesh_.colors.end(),
            regionMesh.colors.begin(),
            regionMesh.colors.end());
    }

    // build water layer
    void buildWater()
    {
        if (waters_.size() == 0) return;

        for (const MeshRegion& region : waters_) {
            Path p(region.points.size());
            for (const Point point : region.points) {
                p.push_back(IntPoint(point.x*Scale, point.y*Scale));
            }
            clipper_.AddPath(p, ptSubject, true);
        }

        Paths solution;
        clipper_.Execute(ctUnion, solution);
        clipper_.Clear();
        waterShape_ = std::move(solution);

        // NOTE we use properties of first region for all water regions
        populateMesh(waters_[0].properties, waterShape_);
    }

    // build road layer
    void buildRoads()
    {
        ClipperOffset offset;
        Paths carRoadPaths = buildOffsetSolution(carRoads_);
        Paths walkRoadsPaths = buildOffsetSolution(walkRoads_);

        clipper_.AddPaths(carRoadPaths, ptClip, true);
        clipper_.AddPaths(walkRoadsPaths, ptSubject, true);
        Paths extrudedWalkRoads;
        clipper_.Execute(ctDifference, extrudedWalkRoads);
        clipper_.Clear();

        carRoadShape_ = std::move(clipRoads(carRoadPaths));
        walkRoadShape_ = std::move(clipRoads(extrudedWalkRoads));

        // NOTE we use properties of first region for road regions
        if (carRoads_.size() > 0)
            populateMesh(carRoads_.begin()->second[0].properties, carRoadShape_);

        if (walkRoads_.size() > 0)
            populateMesh(walkRoads_.begin()->second[0].properties, walkRoadShape_);
    }

    // build surfaces layer
    void buildSurfaces()
    {
        Paths paths = buildPaths(surfaces_);
        for (auto i = 0; i < surfaces_.size(); ++i) {
            clipper_.AddPaths(carRoadShape_, ptClip, true);
            clipper_.AddPaths(walkRoadShape_, ptClip, true);
            clipper_.AddPaths(waterShape_, ptClip, true);
            clipper_.AddPaths(surfaceShape_, ptClip, true);
            clipper_.AddPath(paths[i], ptSubject, true);

            Paths result;
            clipper_.Execute(ctDifference, result, pftPositive, pftPositive);
            clipper_.Clear();

            populateMesh(surfaces_[i].properties, result);

            surfaceShape_.insert(
                surfaceShape_.end(),
                std::make_move_iterator(result.begin()),
                std::make_move_iterator(result.end())
                );
        }
    }

    // build background
    void buildBackground()
    {
        // construct clipRect
        BoundingBox bbox = utymap::index::GeoUtils::quadKeyToBoundingBox(quadKey_);
        Path clipRect;
        clipRect.push_back(IntPoint(bbox.minPoint.longitude*Scale, bbox.minPoint.latitude *Scale));
        clipRect.push_back(IntPoint(bbox.maxPoint.longitude *Scale, bbox.minPoint.latitude *Scale));
        clipRect.push_back(IntPoint(bbox.maxPoint.longitude *Scale, bbox.maxPoint.latitude*Scale));
        clipRect.push_back(IntPoint(bbox.minPoint.longitude*Scale, bbox.maxPoint.latitude*Scale));

        clipper_.AddPath(clipRect, ptSubject, true);

        clipper_.AddPaths(carRoadShape_, ptClip, true);
        clipper_.AddPaths(walkRoadShape_, ptClip, true);
        clipper_.AddPaths(waterShape_, ptClip, true);
        clipper_.AddPaths(surfaceShape_, ptClip, true);
        clipper_.Execute(ctDifference, backgroundShape_, pftPositive, pftPositive);
        clipper_.Clear();

        if (backgroundShape_.size() > 0)
        {
            auto style = styleProvider_.forCanvas(quadKey_.levelOfDetail);
            populateMesh(createMeshRegionProperties(style), backgroundShape_);
        }
    }

const utymap::mapcss::StyleProvider& styleProvider_;
utymap::index::StringTable& stringTable_;
std::function<void(const Mesh&)> callback_;

Clipper clipper_;
ClipperOffset offset_;
LineGridSplitter splitter_;
MeshBuilder meshBuilder_;
QuadKey quadKey_;

MeshRegions waters_;
MeshRegions surfaces_;
RoadMap carRoads_, walkRoads_;
Paths waterShape_, carRoadShape_, walkRoadShape_, surfaceShape_, backgroundShape_;

Mesh mesh_;
};

void TerraBuilder::visitNode(const utymap::entities::Node& node)
{
    pimpl_->visitNode(node);
}

void TerraBuilder::visitWay(const utymap::entities::Way& way)
{
    pimpl_->visitWay(way);
}

void TerraBuilder::visitArea(const utymap::entities::Area& area)
{
    pimpl_->visitArea(area);
}

void TerraBuilder::visitRelation(const utymap::entities::Relation& relation)
{
    pimpl_->visitRelation(relation);
}

void TerraBuilder::prepare(const utymap::QuadKey& quadKey)
{
    pimpl_->setQuadKey(quadKey);
}

void TerraBuilder::complete()
{
    pimpl_->build();
}

TerraBuilder::~TerraBuilder() { }

TerraBuilder::TerraBuilder(utymap::index::StringTable& stringTable,
                           const utymap::mapcss::StyleProvider& styleProvider, 
                           ElevationProvider& eleProvider,
                           std::function<void(const Mesh&)> callback) :
pimpl_(std::unique_ptr<TerraBuilder::TerraBuilderImpl>(new TerraBuilder::TerraBuilderImpl(stringTable, styleProvider, eleProvider, callback)))
{
}
