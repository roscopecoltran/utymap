#include "builders/poi/TreeBuilder.hpp"
#include "utils/MapCssUtils.hpp"

using namespace utymap::builders;
using namespace utymap::mapcss;
using namespace utymap::meshing;
using namespace utymap::utils;

namespace {
const std::string MeshName = "tree";
const std::string FoliageColorKey = "foliage-color";
const std::string TrunkColorKey = "trunk-color";
const std::string FoliageRadius = "foliage-radius";
const std::string TrunkRadius = "trunk-radius";
const std::string TrunkHeight = "trunk-height";
}

void TreeBuilder::visitNode(const utymap::entities::Node& node)
{
    Mesh mesh(MeshName);
    Style style = context_.styleProvider.forElement(node, context_.quadKey.levelOfDetail);
   
    MeshContext meshContext(mesh, style);

    TreeGenerator generator = createGenerator(context_, meshContext);

    double elevation = context_.eleProvider.getElevation(node.coordinate);
    generator
        .setPosition(Vector3(node.coordinate.longitude, elevation, node.coordinate.latitude))
        .generate();

    context_.meshCallback(mesh);
}

TreeGenerator TreeBuilder::createGenerator(const BuilderContext& builderContext, MeshContext& meshContext)
{
    double relativeSize = builderContext.boundingBox.maxPoint.latitude - builderContext.boundingBox.minPoint.latitude;
    GeoCoordinate relativeCoordinate = builderContext.boundingBox.center();

    double foliageRadiusInDegrees = getDimension(FoliageRadius, builderContext.stringTable, meshContext.style, relativeSize, relativeCoordinate);
    double foliageRadiusInMeters = getDimension(FoliageRadius, builderContext.stringTable, meshContext.style, relativeSize) / 2;

    return TreeGenerator(builderContext,
                         meshContext,
                         TrunkColorKey,
                         FoliageColorKey)
        .setFoliageRadius(foliageRadiusInDegrees, foliageRadiusInMeters)
        .setTrunkRadius(getDimension(TrunkRadius, builderContext.stringTable, meshContext.style, relativeSize, relativeCoordinate))
        .setTrunkHeight(getDouble(TrunkHeight, builderContext.stringTable, meshContext.style));
}