#include "builders/terrain/TerraExtras.hpp"
#include "builders/poi/TreeBuilder.hpp"

#include <algorithm>

using namespace utymap::builders;
using namespace utymap::meshing;

void TerraExtras::addForest(const BuilderContext& builderContext, TerraExtras::Context& extrasContext)
{
    // generate tree mesh
    Mesh treeMesh("");
    TreeBuilder::createGenerator(builderContext, MeshContext(treeMesh, extrasContext.style))
        .setPosition(Vector3(0, 0, 0)) // NOTE we will override coordinates later
        .generate();

    // original mesh
    auto& vertices = extrasContext.mesh.vertices;
    auto& triangles = extrasContext.mesh.triangles;
    auto& colors = extrasContext.mesh.colors;

    auto endTriangle = triangles.size();

    // go through mesh region triangles and insert copy of the tree
    for (auto i = extrasContext.startTriangle; i < endTriangle; i += 3) {
        double centroidX = (vertices[i * 3] + vertices[(i + 1) * 3] + vertices[(i + 2) * 3]) / 3;
        double centroidY = (vertices[i * 3 + 1] + vertices[(i + 1) * 3 + 1] + vertices[(i + 2) * 3 + 1]) / 3;
        double elevation = builderContext.eleProvider.getElevation(centroidX, centroidY);

        auto startIndex = static_cast<int>(vertices.size());

        // copy adjusted vertices
        for (std::size_t i = 0; i < treeMesh.vertices.size();) {
            vertices.push_back(treeMesh.vertices[i++] + centroidX);
            vertices.push_back(treeMesh.vertices[i++] + centroidY);
            vertices.push_back(treeMesh.vertices[i++] + elevation);
        }

        // copy adjusted triangles
        std::transform(treeMesh.triangles.begin(), treeMesh.triangles.end(), std::back_inserter(triangles), [&](int value) {
            return value + startIndex;
        });

        // copy adjusted colors
        std::copy(treeMesh.colors.begin(), treeMesh.colors.end(), std::back_inserter(colors));
    }
}

void TerraExtras::addWater(const BuilderContext& builderContext, TerraExtras::Context& eeshContext)
{
    // TODO
}