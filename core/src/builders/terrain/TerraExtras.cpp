#include "builders/terrain/TerraExtras.hpp"
#include "builders/poi/TreeBuilder.hpp"

#include <algorithm>

using namespace utymap::builders;
using namespace utymap::meshing;

void TerraExtras::addForest(const BuilderContext& builderContext, TerraExtras::Context& extrasContext)
{
    // generate tree mesh
    Mesh treeMesh("");
    MeshContext meshContext(treeMesh, extrasContext.style);
    TreeBuilder::createGenerator(builderContext, meshContext)
        .setPosition(Vector3(0, 0, 0)) // NOTE we will override coordinates later
        .generate();

    // forest mesh contains all trees
    Mesh forestMesh("forest");
    
    // go through mesh region triangles and insert copy of the tree
    int step = 3 * 10; // NOTE: only insert in every tenth triangle
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

        double elevation = builderContext.eleProvider.getElevation(centroidX, centroidY);

        int startIndex = static_cast<int>(forestMesh.vertices.size() / 3);

        // copy adjusted vertices
        for (std::size_t i = 0; i < treeMesh.vertices.size();) {
            forestMesh.vertices.push_back(treeMesh.vertices[i++] + centroidX);
            forestMesh.vertices.push_back(treeMesh.vertices[i++] + centroidY);
            forestMesh.vertices.push_back(treeMesh.vertices[i++] + elevation);
        }

        // copy adjusted triangles
        std::transform(treeMesh.triangles.begin(), treeMesh.triangles.end(), std::back_inserter(forestMesh.triangles), [&](int value) {
            return value + startIndex;
        });

        // copy colors
        std::copy(treeMesh.colors.begin(), treeMesh.colors.end(), std::back_inserter(forestMesh.colors));
    }

    builderContext.meshCallback(forestMesh);
}

void TerraExtras::addWater(const BuilderContext& builderContext, TerraExtras::Context& eeshContext)
{
    // TODO
}