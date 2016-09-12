#include "builders/poi/TreeBuilder.hpp"
#include "utils/GradientUtils.hpp"
#include "utils/MeshUtils.hpp"

using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::mapcss;
using namespace utymap::meshing;
using namespace utymap::utils;

namespace {
    const std::string NodeMeshNamePrefix = "tree:";
    const std::string WayMeshNamePrefix = "trees:";

    const std::string TreeStepKey = "tree-step";

    const std::string FoliageColorKey = "foliage-color";
    const std::string TrunkColorKey = "trunk-color";
    const std::string FoliageRadius = "foliage-radius";
    const std::string TrunkRadius = "trunk-radius";
    const std::string TrunkHeight = "trunk-height";
}

void TreeBuilder::visitNode(const utymap::entities::Node& node)
{
    Mesh mesh(utymap::utils::getMeshName(NodeMeshNamePrefix, node));
    Style style = context_.styleProvider.forElement(node, context_.quadKey.levelOfDetail);
    MeshContext meshContext(mesh, style, utymap::mapcss::ColorGradient());

    auto generator = createGenerator(context_, meshContext);

    double elevation = context_.eleProvider.getElevation(node.coordinate);
    generator->setPosition(Vector3(node.coordinate.longitude, elevation, node.coordinate.latitude));
    generator->generate();

    context_.meshCallback(mesh);
}

void TreeBuilder::visitWay(const utymap::entities::Way& way)
{
    Mesh treeMesh("");
    Mesh newMesh(utymap::utils::getMeshName(WayMeshNamePrefix, way));
    Style style = context_.styleProvider.forElement(way, context_.quadKey.levelOfDetail);
    MeshContext meshContext(treeMesh, style, utymap::mapcss::ColorGradient());

    auto generator = TreeBuilder::createGenerator(context_, meshContext);
    generator->setPosition(Vector3(0, 0, 0)); // NOTE we will override coordinates later
    generator->generate();

    double treeStepInMeters = style.getValue(TreeStepKey);

    for (std::size_t i = 0; i < way.coordinates.size() - 1; ++i) {
        const auto& p1 = way.coordinates[i];
        const auto& p2 = way.coordinates[i + 1];

        double distanceInMeters = GeoUtils::distance(p1, p2);
        int treeCount = static_cast<int>(distanceInMeters / treeStepInMeters);

        for (int j = 0; j < treeCount; ++j) {
            GeoCoordinate position = GeoUtils::newPoint(p1, p2, (double) j / treeCount);
            
            double elevation = context_.eleProvider.getElevation(position);
            utymap::utils::copyMesh(Vector3(position.longitude, elevation, position.latitude), treeMesh, newMesh);
        }
    }

    context_.meshCallback(newMesh);
}

void TreeBuilder::visitRelation(const utymap::entities::Relation& relation)
{
    for (const auto& element : relation.elements) {
        element->accept(*this);
    }
}

std::unique_ptr<TreeGenerator> TreeBuilder::createGenerator(const BuilderContext& builderContext, const MeshContext& meshContext)
{
    double relativeSize = builderContext.boundingBox.maxPoint.latitude - builderContext.boundingBox.minPoint.latitude;
    GeoCoordinate relativeCoordinate = builderContext.boundingBox.center();

    double foliageRadiusInDegrees = meshContext.style.getValue(FoliageRadius, relativeSize, relativeCoordinate);
    double foliageRadiusInMeters = meshContext.style.getValue(FoliageRadius, relativeSize);

    const auto& trunkGradient = GradientUtils::evaluateGradient(builderContext.styleProvider, meshContext.style, TrunkColorKey);
    const auto& foliageGradient = GradientUtils::evaluateGradient(builderContext.styleProvider, meshContext.style, FoliageColorKey);

    std::unique_ptr<TreeGenerator> generator(new TreeGenerator(builderContext, meshContext, trunkGradient, foliageGradient));

    generator->setFoliageColorNoiseFreq(0);
    generator->setFoliageRadius(foliageRadiusInDegrees, foliageRadiusInMeters);
    generator->setTrunkColorNoiseFreq(0);
    generator->setTrunkRadius(meshContext.style.getValue(TrunkRadius, relativeSize, relativeCoordinate));
    generator->setTrunkHeight(meshContext.style.getValue(TrunkHeight, relativeSize));

    return generator;
}