#include "builders/poi/TreeBuilder.hpp"
#include "utils/MapCssUtils.hpp"

using namespace utymap::builders;
using namespace utymap::mapcss;
using namespace utymap::meshing;

const std::string TreeBuilder::MeshName = "tree";
const std::string TreeBuilder::FoliageColorKey = "foliage-color";
const std::string TreeBuilder::TrunkColorKey = "trunk-color";

void TreeBuilder::visitNode(const utymap::entities::Node& node)
{
    Mesh mesh(MeshName);
    TreeGenerator generator = createGenerator(mesh, context_, context_.styleProvider.forElement(node, context_.quadKey.levelOfDetail));
    double elevation = context_.eleProvider.getElevation(node.coordinate);
    generator
        .setPosition(Vector3(node.coordinate.longitude, elevation, node.coordinate.latitude))
        .generate();

    context_.meshCallback(mesh);
}

TreeGenerator TreeBuilder::createGenerator(Mesh& mesh, const BuilderContext& context, const Style& style)
{
    auto trunkGradient = utymap::utils::getString(TrunkColorKey, context.stringTable, style);
    auto foliageGradient = utymap::utils::getString(FoliageColorKey, context.stringTable, style);

    // TODO specify all properties in mapcss and read them here if necessary
    return TreeGenerator(mesh, context.meshBuilder, 
            context.styleProvider.getGradient(*trunkGradient),
            context.styleProvider.getGradient(*foliageGradient))
        .setFoliageRadius(2.5)
        .setTrunkRadius(0.2)
        .setTrunkHeight(2);
}