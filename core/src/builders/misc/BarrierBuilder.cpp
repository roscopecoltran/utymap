#include "builders/generators/CylinderGenerator.hpp"
#include "builders/generators/WallGenerator.hpp"
#include "builders/misc/BarrierBuilder.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "mapcss/StyleConsts.hpp"
#include "utils/GradientUtils.hpp"

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::mapcss;
using namespace utymap::math;
using namespace utymap::utils;

namespace {
    const double Scale = 1E7;
    const std::string MeshNamePrefix = "barrier:";
}

void BarrierBuilder::visitNode(const Node& node)
{
    Style style = context_.styleProvider.forElement(node, context_.quadKey.levelOfDetail);
    Mesh mesh(utymap::utils::getMeshName(MeshNamePrefix, node));

    MeshContext meshContext = MeshContext::create(mesh, style, context_.styleProvider,
        StyleConsts::GradientKey(), StyleConsts::TextureIndexKey(),
        StyleConsts::TextureTypeKey(), StyleConsts::TextureScaleKey(), node.id);

    double elevation = context_.eleProvider.getElevation(context_.quadKey, node.coordinate);

    CylinderGenerator generator(context_, meshContext);
    generator
        .setCenter(Vector3(node.coordinate.longitude, elevation, node.coordinate.latitude))
        .setHeight(style.getValue(StyleConsts::HeightKey()))
        .setRadius(style.getValue(StyleConsts::RadiusKey(), context_.boundingBox))
        .setMaxSegmentHeight(5)
        .setRadialSegments(7)
        .setVertexNoiseFreq(0)
        .generate();

    context_.meshBuilder.writeTextureMappingInfo(mesh, meshContext.appearanceOptions);
    context_.meshCallback(mesh);
}

void BarrierBuilder::visitWay(const Way& way)
{
    buildBarrier(way);
}

void BarrierBuilder::visitArea(const Area& area)
{
    buildBarrier(area);
}

template <typename T>
void BarrierBuilder::buildBarrier(const T& element)
{
    Style style = context_.styleProvider.forElement(element, context_.quadKey.levelOfDetail);
    Mesh mesh(utymap::utils::getMeshName(MeshNamePrefix, element));

    MeshContext meshContext = MeshContext::create(mesh, style, context_.styleProvider,
        StyleConsts::GradientKey(), StyleConsts::TextureIndexKey(),
        StyleConsts::TextureTypeKey(), StyleConsts::TextureScaleKey(), element.id);

    double width = style.getValue(StyleConsts::WidthKey(), context_.boundingBox);

    WallGenerator generator(context_, meshContext);
    generator
        .setGeometry(element.coordinates.begin(),
                     element.coordinates.end())
        .setWidth(width)
        .setHeight(style.getValue(StyleConsts::HeightKey()))
        .setLength(style.getValue(StyleConsts::LengthKey()))
        .generate();

    context_.meshBuilder.writeTextureMappingInfo(mesh, meshContext.appearanceOptions);
    context_.meshCallback(mesh);
}