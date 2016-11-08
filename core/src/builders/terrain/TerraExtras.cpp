#include "builders/terrain/TerraExtras.hpp"
#include "builders/poi/TreeBuilder.hpp"
#include "math/Mesh.hpp"
#include "utils/MeshUtils.hpp"

using namespace utymap::builders;
using namespace utymap::math;

namespace {
    const std::string TreeFrequencyKey = "tree-frequency";
}

void TerraExtras::addForest(const BuilderContext& builderContext, TerraExtras::Context& extrasContext)
{
    // generate tree mesh
    Mesh treeMesh("");
    auto generator = TreeBuilder::createGenerator(builderContext, treeMesh, extrasContext.style);
    generator->setPosition(Vector3(0, 0, 0)); // NOTE we will override coordinates later
    generator->generate();

    // forest mesh contains all trees
    Mesh forestMesh("forest");
  
    // go through mesh region triangles and insert copy of the tree
    int step = 3 * static_cast<int>(std::max(extrasContext.style.getValue(TreeFrequencyKey), 1.)); 
    for (auto i = extrasContext.startTriangle; i < extrasContext.mesh.triangles.size(); i += step) {

        double centroidX = 0;
        double centroidY = 0;
        for (int j = 0; j < 3; j++) {
            int index = extrasContext.mesh.triangles[i + j] * 3;
            centroidX += extrasContext.mesh.vertices[index];
            centroidY += extrasContext.mesh.vertices[index + 1];
        }

        centroidX /= 3;
        centroidY /= 3;

        double elevation = builderContext.eleProvider.getElevation(builderContext.quadKey, centroidX, centroidY);

        utymap::utils::copyMesh(Vector3(centroidX, elevation, centroidY), treeMesh, forestMesh);
    }

    builderContext.meshCallback(forestMesh);
}

void TerraExtras::addWater(const BuilderContext& builderContext, TerraExtras::Context& eeshContext)
{
    // TODO
}