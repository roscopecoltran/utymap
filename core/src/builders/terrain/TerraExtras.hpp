#ifndef BUILDERS_TERRAEXTRAS_HPP_DEFINED
#define BUILDERS_TERRAEXTRAS_HPP_DEFINED

#include "builders/BuilderContext.hpp"
#include "meshing/MeshTypes.hpp"

#include <functional>

namespace utymap { namespace builders {

// Provides the way to extend terrain mesh with additional data.
class TerraExtras
{
public:
    // Specifies mesh region which should be used.
    struct MeshContext
    {
        std::size_t startVertex;
        std::size_t startTriangle;
        std::size_t startColor;

        utymap::meshing::Mesh& mesh;
        const utymap::mapcss::Style& style;
        const utymap::meshing::MeshBuilder::Options options;

        MeshContext(utymap::meshing::Mesh& mesh,
                    const utymap::mapcss::Style& style,
                    const utymap::meshing::MeshBuilder::Options& options) :
            startVertex(mesh.vertices.size()),
            startTriangle(mesh.triangles.size()),
            startColor(mesh.colors.size()),
            mesh(mesh), style(style), options(options)
        {
        }
    };

    // Specifies Extras function signature.
    typedef std::function<void(const utymap::builders::BuilderContext&, MeshContext&)> ExtrasFunc;

    // Extends mesh with trees.
    static void addForest(const utymap::builders::BuilderContext& builderContext, MeshContext& meshContext);

    // Extends mesh with water surface.
    static void addWater(const utymap::builders::BuilderContext& builderContext,  MeshContext& meshContext);
};

}}
#endif // BUILDERS_TERRAEXTRAS_HPP_DEFINED
