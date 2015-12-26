#include "meshing/clipper.hpp"
#include "meshing/Polygon.hpp"
#include "meshing/MeshBuilder.hpp"
#include "terrain/LineGridSplitter.hpp"
#include "terrain/TerraBuilder.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <map>
#include <unordered_map>
#include <vector>

using namespace ClipperLib;
using namespace utymap::heightmap;
using namespace utymap::meshing;
using namespace utymap::terrain;

const uint64_t Scale = 1E8;
const uint64_t DoubleScale = Scale * Scale;

typedef std::vector<MeshRegion> MeshRegions;
typedef std::unordered_map<int, MeshRegions> RoadMap;
typedef std::map<int, MeshRegions> SurfaceMap;
typedef std::vector<Point<double>> MeshPoints;

class TerraBuilder::TerraBuilderImpl
{
public:

    TerraBuilderImpl(ElevationProvider<double>& eleProvider) :
        meshBuilder_(eleProvider)
    {
    }

    inline void addWater(const MeshRegion& water)
    {
        waters_.push_back(water);
    }

    inline void addSurface(const MeshRegion& surface)
    {
        surfaces_[surface.properties.gradientKey].push_back(surface);
    }

    inline void addCarRoad(const MeshRegion& carRoad, int width)
    {
        carRoads_[width].push_back(carRoad);
    }

    inline void addWalkRoad(const MeshRegion& walkRoad, int width)
    {
        walkRoads_[width].push_back(walkRoad);
    }

    inline void setBackgroundProperties(const MeshRegion::Properties& properties)
    {
        bgProperties_ = properties;
    }

    // builds tile mesh using data provided.
    utymap::meshing::Mesh<double> build(const utymap::meshing::Rectangle<double>& tileRect, int levelOfDetails)
    {
        clipRect_ = createPathFromRect(tileRect);
        splitter_.setRoundDigits(7);
        splitter_.setScale(Scale);

        // fill context with layer specific data.
        buildWater();
        buildRoads();
        buildSurfaces();
        buildBackground();

        return std::move(mesh_);
    }
private:

    Path createPathFromRect(const Rectangle<double>& tileRect)
    {
        Path rect;
        rect.push_back(IntPoint(tileRect.xMin*Scale, tileRect.yMin*Scale));
        rect.push_back(IntPoint(tileRect.xMax*Scale, tileRect.yMin*Scale));
        rect.push_back(IntPoint(tileRect.xMax*Scale, tileRect.yMax*Scale));
        rect.push_back(IntPoint(tileRect.xMin*Scale, tileRect.yMax*Scale));

        return std::move(rect);
    }

    Paths clipByRect(const Paths& subjects)
    {
        Paths solution;
        clipper_.AddPaths(subjects, ptSubject, true);
        clipper_.AddPath(clipRect_, ptClip, true);
        clipper_.Execute(ctIntersection, solution);
        clipper_.Clear();
        return std::move(solution);
    }

    Paths buildPaths(const MeshRegions& regions) {
        // TODO holes are not processed
        Paths paths(regions.size());
        for (const MeshRegion& region : regions) {
            Path p(region.points.size());
            for (const Point<double> point : region.points) {
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
        return std::move(clipByRect(resultRoads));
    }

    // populates mesh 
    void populateMesh(const MeshRegion::Properties& properties, const Paths& paths)
    {
        bool hasHeightOffset = properties.heightOffset > 0;
        // TODO precalculate capacity somehow?
        Polygon<double> polygon(256, 1);
        std::vector<MeshPoints> contours(hasHeightOffset ? 4 : 0);

        for (Path path : paths) {
            double area = ClipperLib::Area(path);

            // TODO skip small polygons to prevent triangulation issues?
            //if (std::abs(area / DoubleScale) < 0.0000001) continue;

            MeshPoints points = restorePoints(path);
            if (area < 0)
                polygon.addHole(points);
            else
                polygon.addContour(points);

            if (hasHeightOffset)
                contours.push_back(points);
        }

        if (hasHeightOffset) {
            std::reverse(contours.begin(), contours.end());
            processHeightOffset(contours);
        }

        fillMesh(properties, polygon);
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

    void fillMesh(const MeshRegion::Properties& properties, Polygon<double>& polygon)
    {
        // TODO use valid area value
        Mesh<double> regionMesh = meshBuilder_.build(polygon, MeshBuilder::Options
        {
            /* area=*/ 1,
            /* elevation noise frequency*/ properties.eleNoiseFreq,
            /* segmentSplit=*/ 0
        });

        // TODO fill mesh colors based on region properties

        if (properties.action != nullptr)
            properties.action(regionMesh);

        mesh_.vertices.insert(mesh_.vertices.begin(),
            regionMesh.vertices.begin(),
            regionMesh.vertices.end());

        mesh_.triangles.insert(mesh_.triangles.begin(),
            regionMesh.triangles.begin(),
            regionMesh.triangles.end());

        mesh_.colors.insert(mesh_.colors.begin(),
            regionMesh.colors.begin(),
            regionMesh.colors.end());
    }

    // build water layer
    void buildWater()
    {
        if (waters_.size() == 0) return;

        for (const MeshRegion& region : waters_) {
            Path p(region.points.size());
            for (const Point<double> point : region.points) {
                p.push_back(IntPoint(point.x*Scale, point.y*Scale));
            }
            clipper_.AddPath(p, ptSubject, true);
        }

        Paths solution;
        clipper_.Execute(ctUnion, solution);
        clipper_.Clear();
        waterShape_ = std::move(clipByRect(solution));

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
        for (auto surfacePair : surfaces_) {
            Paths paths = buildPaths(surfacePair.second);
            clipper_.AddPaths(paths, ptSubject, true);
            Paths surfacesUnion;
            clipper_.Execute(ctUnion, surfacesUnion);
            clipper_.Clear();

            clipper_.AddPaths(carRoadShape_, ptClip, true);
            clipper_.AddPaths(walkRoadShape_, ptClip, true);
            clipper_.AddPaths(waterShape_, ptClip, true);
            clipper_.AddPaths(surfaceShape_, ptClip, true);
            clipper_.AddPaths(surfacesUnion, ptSubject, true);
            Paths result;
            clipper_.Execute(ctDifference, result, pftPositive, pftPositive);
            clipper_.Clear();
            result = clipByRect(result);

            // NOTE surfaces are merged by gradient key.
            populateMesh(surfacePair.second[0].properties, result);

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
        clipper_.AddPath(clipRect_, ptSubject, true);

        clipper_.AddPaths(carRoadShape_, ptClip, true);
        clipper_.AddPaths(walkRoadShape_, ptClip, true);
        clipper_.AddPaths(waterShape_, ptClip, true);
        clipper_.AddPaths(surfaceShape_, ptClip, true);
        clipper_.Execute(ctDifference, backgroundShape_, pftPositive, pftPositive);
        clipper_.Clear();

        if (backgroundShape_.size() > 0)
            populateMesh(bgProperties_, backgroundShape_);
    }

// mesh builder
MeshBuilder meshBuilder_;

// input data
MeshRegions waters_;
SurfaceMap surfaces_;
RoadMap carRoads_;
RoadMap walkRoads_;
MeshRegion::Properties bgProperties_;

// clipper data
Clipper clipper_;
ClipperOffset offset_;
Path clipRect_;
Paths waterShape_;
Paths carRoadShape_;
Paths walkRoadShape_;
Paths surfaceShape_;
Paths backgroundShape_;

// splits segments using grid
LineGridSplitter<double> splitter_;

// output
Mesh<double> mesh_;
};

void TerraBuilder::addWater(const MeshRegion& water)
{
    pimpl_->addWater(water);
}

void TerraBuilder::addSurface(const MeshRegion& surface)
{
    pimpl_->addSurface(surface);
}

void TerraBuilder::addCarRoad(const MeshRegion& carRoad, int width)
{
    pimpl_->addCarRoad(carRoad, width);
}

void TerraBuilder::addWalkRoad(const MeshRegion& walkRoad, int width)
{
    pimpl_->addWalkRoad(walkRoad, width);
}

Mesh<double> TerraBuilder::build(const Rectangle<double>& tileRect, int levelOfDetails)
{
    return std::move(pimpl_->build(tileRect, levelOfDetails));
}

void TerraBuilder::setBackgroundProperties(const MeshRegion::Properties& properties)
{
    pimpl_->setBackgroundProperties(properties);
}

TerraBuilder::TerraBuilder(ElevationProvider<double>& eleProvider) :
pimpl_(std::unique_ptr<TerraBuilder::TerraBuilderImpl>(new TerraBuilder::TerraBuilderImpl(eleProvider)))
{
}

TerraBuilder::~TerraBuilder() { }
