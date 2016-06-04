#include "builders/poi/TreeBuilder.hpp"
#include "utils/ElementUtils.hpp"
#include "utils/GradientUtils.hpp"

using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::mapcss;
using namespace utymap::meshing;
using namespace utymap::utils;

namespace {
    const std::string MeshNamePrefix = "tree:";
    const std::string FoliageColorKey = "foliage-color";
    const std::string TrunkColorKey = "trunk-color";
    const std::string FoliageRadius = "foliage-radius";
    const std::string TrunkRadius = "trunk-radius";
    const std::string TrunkHeight = "trunk-height";
}

void TreeBuilder::visitNode(const utymap::entities::Node& node)
{
    Mesh mesh(utymap::utils::getMeshName(MeshNamePrefix, node));
    Style style = context_.styleProvider.forElement(node, context_.quadKey.levelOfDetail);
   
    MeshContext meshContext(mesh, style);

    TreeGenerator generator = createGenerator(context_, meshContext, node.tags);

    double elevation = context_.eleProvider.getElevation(node.coordinate);
    generator
        .setPosition(Vector3(node.coordinate.longitude, elevation, node.coordinate.latitude))
        .generate();

    context_.meshCallback(mesh);
}

TreeGenerator TreeBuilder::createGenerator(const BuilderContext& builderContext, MeshContext& meshContext, const std::vector<Tag>& tags)
{
    double relativeSize = builderContext.boundingBox.maxPoint.latitude - builderContext.boundingBox.minPoint.latitude;
    GeoCoordinate relativeCoordinate = builderContext.boundingBox.center();

    double foliageRadiusInDegrees = meshContext.style.getValue(FoliageRadius, relativeSize, relativeCoordinate);
    double foliageRadiusInMeters = meshContext.style.getValue(FoliageRadius, relativeSize);

    auto foliageGradient = GradientUtils::evaluateGradient(builderContext.styleProvider, meshContext.style, tags, FoliageColorKey);
    auto trunkGradient = GradientUtils::evaluateGradient(builderContext.styleProvider, meshContext.style, tags, TrunkColorKey);

    return TreeGenerator(builderContext, meshContext)
        .setFoliageColor(foliageGradient, 0)
        .setFoliageRadius(foliageRadiusInDegrees, foliageRadiusInMeters)
        .setTrunkColor(trunkGradient, 0)
        .setTrunkRadius(meshContext.style.getValue(TrunkRadius, relativeSize, relativeCoordinate))
        .setTrunkHeight(meshContext.style.getValue(TrunkHeight, relativeSize));
}