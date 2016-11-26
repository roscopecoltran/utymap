#include "builders/poi/TreeBuilder.hpp"
#include "utils/MeshUtils.hpp"

using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::mapcss;
using namespace utymap::math;
using namespace utymap::utils;

namespace {
    const std::string NodeMeshNamePrefix = "tree:";
    const std::string WayMeshNamePrefix = "trees:";

    const std::string TreeStepKey = "tree-step";

    const std::string FoliageColorKey = "foliage-color";
    const std::string FoliageRadius = "foliage-radius";
    const std::string FoliageTextureIndexKey = "foliage-texture-index";
    const std::string FoliageTextureTypeKey = "foliage-texture-type";
    const std::string FoliageTextureScaleKey = "foliage-texture-scale";

    const std::string TrunkColorKey = "trunk-color";
    const std::string TrunkRadius = "trunk-radius";
    const std::string TrunkHeight = "trunk-height";
    const std::string TrunkTextureIndexKey = "trunk-texture-index";
    const std::string TrunkTextureTypeKey = "trunk-texture-type";
    const std::string TrunkTextureScaleKey = "trunk-texture-scale";

    /// Gets a reference to texture region using parameters provided.
    const TextureRegion& getTextureRegion(const StyleProvider& styleProvider, 
                                          const Style& style,
                                          const std::string textureIndexKey,
                                          const std::string textureTypeKey)
    {
        return styleProvider
            .getTexture(static_cast<std::uint16_t>(style.getValue(textureIndexKey)),
                        style.getString(textureTypeKey))
            .random(0);
    }
}

void TreeBuilder::visitNode(const utymap::entities::Node& node)
{
    Mesh mesh(utymap::utils::getMeshName(NodeMeshNamePrefix, node));
    Style style = context_.styleProvider.forElement(node, context_.quadKey.levelOfDetail);
    
    auto generator = createGenerator(context_, mesh, style);

    double elevation = context_.eleProvider.getElevation(context_.quadKey, node.coordinate);
    generator->setPosition(Vector3(node.coordinate.longitude, elevation, node.coordinate.latitude));
    generator->generate();

    context_.meshCallback(mesh);
}

void TreeBuilder::visitWay(const utymap::entities::Way& way)
{
    Mesh treeMesh("");
    Mesh newMesh(utymap::utils::getMeshName(WayMeshNamePrefix, way));
    Style style = context_.styleProvider.forElement(way, context_.quadKey.levelOfDetail);

    auto generator = TreeBuilder::createGenerator(context_, treeMesh, style);
    generator->setPosition(Vector3(0, 0, 0)); // NOTE we will override coordinates later
    generator->generate();

    double treeStepInMeters = style.getValue(TreeStepKey);

    for (std::size_t i = 0; i < way.coordinates.size() - 1; ++i) {
        const auto& p0 = way.coordinates[i];
        const auto& p1 = way.coordinates[i + 1];
        utymap::utils::copyMeshAlong(context_.quadKey, p0, p1, treeMesh, newMesh, treeStepInMeters, context_.eleProvider);
    }

    context_.meshCallback(newMesh);
}

void TreeBuilder::visitRelation(const utymap::entities::Relation& relation)
{
    for (const auto& element : relation.elements) {
        element->accept(*this);
    }
}

std::unique_ptr<TreeGenerator> TreeBuilder::createGenerator(const BuilderContext& builderContext, Mesh& mesh, const Style& style)
{
    double relativeSize = builderContext.boundingBox.maxPoint.latitude - builderContext.boundingBox.minPoint.latitude;
    GeoCoordinate relativeCoordinate = builderContext.boundingBox.center();

    double foliageRadiusInDegrees = style.getValue(FoliageRadius, relativeSize, relativeCoordinate);
    double foliageRadiusInMeters = style.getValue(FoliageRadius, relativeSize);

    const auto& trunkGradient = GradientUtils::evaluateGradient(builderContext.styleProvider, style, TrunkColorKey);
    const auto& foliageGradient = GradientUtils::evaluateGradient(builderContext.styleProvider, style, FoliageColorKey);

    const auto& trunkTexture = getTextureRegion(builderContext.styleProvider, style, TrunkTextureIndexKey, TrunkTextureTypeKey);
    const auto& foliageTexture = getTextureRegion(builderContext.styleProvider, style, FoliageTextureIndexKey, FoliageTextureTypeKey);

    auto generator = utymap::utils::make_unique<TreeGenerator>(builderContext, mesh, style, 
        trunkGradient, foliageGradient, trunkTexture, foliageTexture);

    generator->setFoliageColorNoiseFreq(0);
    generator->setFoliageSize(Vector3(1.5 * foliageRadiusInDegrees, foliageRadiusInMeters, foliageRadiusInDegrees));
    generator->setFoliageTextureScale(style.getValue(FoliageTextureScaleKey));
    generator->setTrunkColorNoiseFreq(0);
    generator->setTrunkRadius(style.getValue(TrunkRadius, relativeSize, relativeCoordinate));
    generator->setTrunkHeight(style.getValue(TrunkHeight, relativeSize));
    generator->setTrunkTextureScale(style.getValue(TrunkTextureScaleKey));

    return generator;
}