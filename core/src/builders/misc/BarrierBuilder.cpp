#include "builders/buildings/roofs/FlatRoofBuilder.hpp"
#include "builders/buildings/facades/FlatFacadeBuilder.hpp"
#include "builders/misc/BarrierBuilder.hpp"
#include "clipper/clipper.hpp"
#include "entities/Way.hpp"
#include "utils/ElementUtils.hpp"
#include "utils/GeometryUtils.hpp"
#include "utils/GeoUtils.hpp"

using namespace ClipperLib;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::mapcss;
using namespace utymap::meshing;
using namespace utymap::utils;

namespace {
    const double Scale = 1E7;
    const std::string HeightKey = "height";
    const std::string MinHeightKey = "min-height";
    const std::string OffsetKey = "offset";
    const std::string MeshNamePrefix = "barrier:";
}

void BarrierBuilder::visitWay(const Way& way)
{
    Style style = context_.styleProvider.forElement(way, context_.quadKey.levelOfDetail);

    ClipperOffset offset;
    Path path;
    path.reserve(way.coordinates.size());

    for (const auto& coord : way.coordinates) {
        path.push_back(IntPoint(coord.longitude * Scale, coord.latitude * Scale));
    }

    if (path[0] == path[path.size() - 1])
        path.pop_back();

    offset.AddPath(path, JoinType::jtMiter, EndType::etOpenSquare);

    Paths solution;
    double offsetInMeters = style.getValue(OffsetKey, way.tags);
    double offsetInGrads = GeoUtils::getOffset(way.coordinates[0], offsetInMeters);
    offset.Execute(solution, offsetInGrads * Scale);
    auto& shape = solution[0];

    // NOTE ensure proper orientation which is required by facade builder
    // used below
    if (ClipperLib::Orientation(shape))
        std::reverse(shape.begin(), shape.end());

    // get polygon
    Polygon polygon(shape.size(), 0);
    std::vector<Vector2> vertices;
    vertices.reserve(shape.size());
    for (const auto& p : shape) {
        vertices.push_back(Vector2(p.X / Scale, p.Y / Scale));
    }

    polygon.addContour(vertices);

    buildFromPolygon(way, style, polygon);
}

void BarrierBuilder::buildFromPolygon(const Way& way, const Style& style, Polygon& polygon)
{
    double height = style.getValue(HeightKey, way.tags);
    double minHeight = style.getValue(MinHeightKey, way.tags);
    double elevation = context_.eleProvider.getElevation(way.coordinates[0]) + minHeight;

    Mesh mesh(utymap::utils::getMeshName(MeshNamePrefix, way));
    MeshContext meshContext(mesh, style);

    // NOTE: Reuse building builders.

    FlatRoofBuilder roofBuilder(context_, meshContext);
    roofBuilder.setHeight(0);
    roofBuilder.setMinHeight(elevation + height);
    roofBuilder.build(polygon);

    FlatFacadeBuilder facadeBuilder(context_, meshContext);
    facadeBuilder.setHeight(height);
    facadeBuilder.setMinHeight(elevation);
    facadeBuilder.build(polygon);

    context_.meshCallback(mesh);
}