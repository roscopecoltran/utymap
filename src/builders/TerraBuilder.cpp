#include "BoundingBox.hpp"
#include "clipper/clipper.hpp"
#include "builders/TerraBuilder.hpp"
#include "meshing/Polygon.hpp"
#include "meshing/MeshBuilder.hpp"
#include "meshing/LineGridSplitter.hpp"
#include "index/GeoUtils.hpp"

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

const uint64_t Scale = 1E8; // max precision for Lat/Lon
const uint64_t DoubleScale = Scale * Scale;

// Represents terrain region.
struct MeshRegion
{
    struct Properties
    {
        const int NoValue = -1;

        int gradientKey;
        int textureAtlas;
        int textureKey;
        float eleNoiseFreq;
        float colorNoiseFreq;
        float heightOffset;

        // Specific mesh action associated with given region.
        std::function<void(utymap::meshing::Mesh<double>)> action;

        Properties() :
            gradientKey(NoValue),
            textureAtlas(NoValue),
            textureKey(NoValue),
            eleNoiseFreq(0),
            colorNoiseFreq(0),
            heightOffset(0),
            action(nullptr)
        {
        }

        Properties& operator =(const Properties & obj)
        {
            if (this != &obj)
            {
                gradientKey = obj.gradientKey;
                textureAtlas = obj.textureAtlas;
                textureKey = obj.textureKey;
                eleNoiseFreq = obj.eleNoiseFreq;
                colorNoiseFreq = obj.colorNoiseFreq;
                heightOffset = obj.heightOffset;
                action = obj.action;
            }
            return *this;
        }
    };

    utymap::meshing::Contour<double> points;
    std::vector<utymap::meshing::Contour<double>> holes;
    Properties properties;
};

typedef std::vector<MeshRegion> MeshRegions;
typedef std::unordered_map<int, MeshRegions> RoadMap;
typedef std::map<int, MeshRegions> SurfaceMap;
typedef std::vector<Point<double>> MeshPoints;

class TerraBuilder::TerraBuilderImpl
{
    const std::string BackgroundColorNoiseFreq = "background-color-noise-freq";
    const std::string BackgroundEleNoiseFreq = "background-ele-noise-freq";
public:

    TerraBuilderImpl(utymap::index::StringTable& stringTable,
                     const utymap::mapcss::StyleProvider& styleProvider,
                     ElevationProvider<double>& eleProvider,
                     const std::function<void(const Mesh<double>&)>& callback) :
        stringTable_(stringTable),
        styleProvider_(styleProvider),
        meshBuilder_(eleProvider),
        callback_(callback),
        quadKey_()
    {
    }

    inline void setQuadKey(const QuadKey& quadKey)
    {
        quadKey_ = quadKey;
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

    // builds tile mesh using data provided.
    void build()
    {
        //clipRect_ = createPathFromRect(tileRect);
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
        int roundDigits = 1;
        int coeff = 1;

        // TODO
        switch (levelOfDetails)
        {
            case 1: roundDigits = 7; break;
            default:
                throw std::domain_error("Unknown LoD");
        };

        splitter_.setRoundDigits(roundDigits, coeff);
        splitter_.setScale(Scale);
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
        return std::move(resultRoads);
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
            MeshRegion::Properties properties;
            properties.eleNoiseFreq = std::stof(getStyleValue(BackgroundEleNoiseFreq, style));
            properties.colorNoiseFreq = std::stof(getStyleValue(BackgroundColorNoiseFreq, style));
            populateMesh(properties, backgroundShape_);
        }
    }

    inline std::string& getStyleValue(const std::string& key, const Style& style)
    {
        uint32_t keyId = stringTable_.getId(key);
        return style.get(keyId);
    }

const utymap::mapcss::StyleProvider& styleProvider_;
utymap::index::StringTable& stringTable_;
const std::function<void(const Mesh<double>&)>& callback_;

// mesh builder
MeshBuilder meshBuilder_;

QuadKey quadKey_;

// input data
MeshRegions waters_;
SurfaceMap surfaces_;
RoadMap carRoads_;
RoadMap walkRoads_;

// clipper data
Clipper clipper_;
ClipperOffset offset_;

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

void TerraBuilder::visitNode(const utymap::entities::Node& node)
{
}

void TerraBuilder::visitWay(const utymap::entities::Way& way)
{
}

void TerraBuilder::visitArea(const utymap::entities::Area& area)
{
}

void TerraBuilder::visitRelation(const utymap::entities::Relation& relation)
{
}

void TerraBuilder::prepare(const utymap::QuadKey& quadKey)
{
    pimpl_->setQuadKey(quadKey);
}

void TerraBuilder::complete()
{
    pimpl_->build();
}

/*void TerraBuilder::addWater(const MeshRegion& water)
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
}*/

TerraBuilder::~TerraBuilder() { }

TerraBuilder::TerraBuilder(utymap::index::StringTable& stringTable,
                           const utymap::mapcss::StyleProvider& styleProvider, 
                           ElevationProvider<double>& eleProvider,
                           const std::function<void(const Mesh<double>&)>& callback) :
pimpl_(std::unique_ptr<TerraBuilder::TerraBuilderImpl>(new TerraBuilder::TerraBuilderImpl(stringTable, styleProvider, eleProvider, callback)))
{
}
