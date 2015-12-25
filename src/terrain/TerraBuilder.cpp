#include "meshing/clipper.hpp"
#include "terrain/LineGridSplitter.hpp"
#include "terrain/TerraBuilder.hpp"

#include <cmath>
#include <cstdint>
#include <iterator>
#include <map>
#include <unordered_map>
#include <vector>

using namespace utymap::meshing;
using namespace utymap::terrain;
using namespace ClipperLib;

const uint64_t Scale = 1E8;
const uint64_t DoubleScale = Scale * Scale;

typedef std::vector<MeshRegion> MeshRegions;
typedef std::unordered_map<int, MeshRegions> RoadMap;
typedef std::map<int, MeshRegions> SurfaceMap;

class TerraBuilder::TerraBuilderImpl
{
public:

    TerraBuilderImpl()
    {
    }

    // Adds water region to tile mesh.
    inline void addWater(const MeshRegion& water)
    {
        waters_.push_back(water);
    }

    // Adds surface region to tile mesh. 
    // Regions will be sorted and merged using gradient key as reference.
    inline void addSurface(const MeshRegion& surface)
    {
        surfaces_[surface.gradientKey].push_back(surface);
    }

    // Add car road region to tile mesh.
    inline void addCarRoad(const MeshRegion& carRoad, int width)
    {
        carRoads_[width].push_back(carRoad);
    }

    // Add walk road region to tile mesh.
    inline void addWalkRoad(const MeshRegion& walkRoad, int width)
    {
        walkRoads_[width].push_back(walkRoad);
    }

    // builds tile mesh using data provided.
    utymap::meshing::Mesh<double> build(const utymap::meshing::Rectangle<double>& tileRect, int levelOfDetails)
    {
        clipRect_ = createPathFromRect(tileRect);

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
        rect.push_back(IntPoint(tileRect.xMin*Scale, tileRect.yMin*Scale));
        rect.push_back(IntPoint(tileRect.xMin*Scale, tileRect.yMin*Scale));
        rect.push_back(IntPoint(tileRect.xMin*Scale, tileRect.yMin*Scale));

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

    void populateMesh(const MeshRegion& region, const Paths& paths)
    {
        for (Path path : paths) {
            double area = ClipperLib::Area(path);
            // skip small polygons to prevent triangulation issues
            if (std::abs(area / DoubleScale) < 0.001) continue;

            // TODO
        }
    }

    std::vector<Point<double>> restorePoints(Path path)
    {
        // TODO
        int lastItemIndex = path.size() - 1;

        /*
        for (int i = 0; i <= lastItemIndex; i++)
        {
        var start = path[i];
        var end = path[i == lastItemIndex ? 0 : i + 1];

        var p1 = new Point(Math.Round(start.X/Scale, MathUtils.RoundDigitCount),
        Math.Round(start.Y/Scale, MathUtils.RoundDigitCount));

        var p2 = new Point(Math.Round(end.X/Scale, MathUtils.RoundDigitCount),
        Math.Round(end.Y/Scale, MathUtils.RoundDigitCount));

        if (isOverview &&
        (!rect.IsOnBorder(new Vector2d(p1.X, p1.Y)) ||
        !rect.IsOnBorder(new Vector2d(p2.X, p2.Y))))
        {
        points.Add(p1);
        continue;
        }

        _lineGridSplitter.Split(p1, p2, _objectPool, points);
        }
        */
    }


    // build water layer
    void buildWater()
    {
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

            // TODO process surfaces as context is present only here

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
    }

MeshRegions waters_;
SurfaceMap surfaces_;
RoadMap carRoads_;
RoadMap walkRoads_;

Clipper clipper_;
ClipperOffset offset_;
Path clipRect_;
Paths waterShape_;
Paths carRoadShape_;
Paths walkRoadShape_;
Paths surfaceShape_;
Paths backgroundShape_;

LineGridSplitter<double> splitter_;

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

TerraBuilder::TerraBuilder() :
pimpl_(std::unique_ptr<TerraBuilder::TerraBuilderImpl>(new TerraBuilder::TerraBuilderImpl()))
{
}

TerraBuilder::~TerraBuilder() { }
