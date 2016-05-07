#include "builders/poi/TreeBuilder.hpp"
#include "utils/MapCssUtils.hpp"

using namespace utymap::builders;
using namespace utymap::mapcss;
using namespace utymap::meshing;
using namespace utymap::utils;

const std::string TreeBuilder::MeshName = "tree";
const std::string TreeBuilder::FoliageColorKey = "foliage-color";
const std::string TreeBuilder::TrunkColorKey = "trunk-color";
const std::string TreeBuilder::FoliageRadius = "foliage-radius";
const std::string TreeBuilder::TrunkRadius = "trunk-radius";
const std::string TreeBuilder::TrunkHeight = "trunk-height";

void TreeBuilder::visitNode(const utymap::entities::Node& node)
{
    Mesh mesh(MeshName);
    TreeGenerator generator = createGenerator(context_, mesh, 
        context_.styleProvider.forElement(node, context_.quadKey.levelOfDetail));

    double elevation = context_.eleProvider.getElevation(node.coordinate);
    generator
        .setPosition(Vector3(node.coordinate.longitude, elevation, node.coordinate.latitude))
        .generate();

    context_.meshCallback(mesh);
}

TreeGenerator TreeBuilder::createGenerator(const BuilderContext& context, Mesh& mesh, const Style& style)
{
    auto trunkGradient = utymap::utils::getString(TrunkColorKey, context.stringTable, style);
    auto foliageGradient = utymap::utils::getString(FoliageColorKey, context.stringTable, style);

    double relativeSize = context.boundingBox.maxPoint.latitude - context.boundingBox.minPoint.latitude;
    GeoCoordinate relativeCoordinate = context.boundingBox.center();

    return TreeGenerator(mesh, context.meshBuilder, 
            context.styleProvider.getGradient(*trunkGradient),
            context.styleProvider.getGradient(*foliageGradient))
        .setFoliageRadius(getDimension(FoliageRadius, context.stringTable, style, relativeSize, relativeCoordinate))
        .setTrunkRadius(getDimension(TrunkRadius, context.stringTable, style, relativeSize, relativeCoordinate))
        .setTrunkHeight(getDimension(TrunkHeight, context.stringTable, style, relativeSize, relativeCoordinate));
}