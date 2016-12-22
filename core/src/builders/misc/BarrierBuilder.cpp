#include "builders/generators/CylinderGenerator.hpp"
#include "builders/generators/WallGenerator.hpp"
#include "builders/misc/BarrierBuilder.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "mapcss/StyleConsts.hpp"
#include "utils/GradientUtils.hpp"
#include "utils/MeshUtils.hpp"

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::mapcss;
using namespace utymap::math;
using namespace utymap::utils;

namespace {
    const double Scale = 1E7;
    const std::string MeshNamePrefix = "barrier:";
    const std::string PillarType = "pillar";
}

void BarrierBuilder::visitNode(const Node& node)
{
    Style style = context_.styleProvider.forElement(node, context_.quadKey.levelOfDetail);
    Mesh mesh(utymap::utils::getMeshName(MeshNamePrefix, node));
    MeshContext meshContext = MeshContext::create(mesh, style, context_.styleProvider, node.id);

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

    context_.meshCallback(mesh);
}

void BarrierBuilder::visitWay(const Way& way)
{
    build(way);
}

void BarrierBuilder::visitArea(const Area& area)
{
    build(area);
}

void BarrierBuilder::visitRelation(const Relation& relation)
{
    for (const auto& element : relation.elements)
        element->accept(*this);
}

template <typename T>
void BarrierBuilder::build(const T& element)
{
    Style style = context_.styleProvider.forElement(element, context_.quadKey.levelOfDetail);
    Mesh mesh(utymap::utils::getMeshName(MeshNamePrefix, element));
    MeshContext meshContext = MeshContext::create(mesh, style, context_.styleProvider, element.id);

    if (style.getString(StyleConsts::TypeKey()) == PillarType)
        buildPillar(element, meshContext);
    else
        buildWall(element, meshContext);
}

template <typename T>
void BarrierBuilder::buildWall(const T& element, MeshContext& meshContext)
{
    double width = meshContext.style.getValue(StyleConsts::WidthKey(), context_.boundingBox);
    WallGenerator generator(context_, meshContext);
    generator
        .setGeometry(element.coordinates.begin(),
                     element.coordinates.end())
        .setWidth(width)
        .setHeight(meshContext.style.getValue(StyleConsts::HeightKey()))
        .setLength(meshContext.style.getValue(StyleConsts::LengthKey()))
        .setGap(meshContext.style.getValue(StyleConsts::GapKey()))
        .generate();
    context_.meshCallback(meshContext.mesh);
}

template <typename T>
void BarrierBuilder::buildPillar(const T& element, MeshContext& meshContext)
{
    Mesh mesh(utymap::utils::getMeshName(MeshNamePrefix, element));
    CylinderGenerator generator(context_, meshContext);
    generator
        .setCenter(Vector3(0, 0, 0))
        .setHeight(meshContext.style.getValue(StyleConsts::HeightKey()))
        .setRadius(meshContext.style.getValue(StyleConsts::RadiusKey(), context_.boundingBox))
        .setMaxSegmentHeight(5)
        .setRadialSegments(7)
        .setVertexNoiseFreq(0)
        .generate();
    
    double treeStepInMeters = meshContext.style.getValue(StyleConsts::StepKey());
    for (std::size_t i = 0; i < element.coordinates.size() - 1; ++i) {
        const auto& p0 = element.coordinates[i];
        const auto& p1 = element.coordinates[i + 1];
        utymap::utils::copyMeshAlong(context_.quadKey, p0, p1, meshContext.mesh, mesh, treeStepInMeters, context_.eleProvider);
    }

    context_.meshCallback(mesh);
}
