#ifndef BUILDERS_BUILDERCONTEXT_HPP_DEFINED
#define BUILDERS_BUILDERCONTEXT_HPP_DEFINED

#include "QuadKey.hpp"
#include "heightmap/ElevationProvider.hpp"
#include "mapcss/StyleProvider.hpp"
#include "meshing/MeshBuilder.hpp"
#include "meshing/MeshTypes.hpp"

#include <functional>
#include <string>
#include <memory>

namespace utymap { namespace builders {

// Provides all dependencies consumed by element builders.
struct BuilderContext
{
    // Current quadkey.
    const utymap::QuadKey& quadKey;
    // Current style provider.
    const utymap::mapcss::StyleProvider& styleProvider;
    // String table.
    utymap::index::StringTable& stringTable;
    // Mesh callback should be called once mesh is constructed.
    std::function<void(const utymap::meshing::Mesh&)> meshCallback;
    // Element callback is called to process original element by external logic.
    std::function<void(const utymap::entities::Element&)> elementCallback;
    // Mesh builder.
    const utymap::meshing::MeshBuilder meshBuilder;

    BuilderContext(const utymap::QuadKey& quadKey,
                   const utymap::mapcss::StyleProvider& styleProvider,
                   utymap::index::StringTable& stringTable,
                   const utymap::heightmap::ElevationProvider& eleProvider,
                   std::function<void(const utymap::meshing::Mesh&)> meshCallback,
                   std::function<void(const utymap::entities::Element&)> elementCallback) :
        quadKey(quadKey), 
        styleProvider(styleProvider),
        stringTable(stringTable),
        meshBuilder(eleProvider),
        meshCallback(meshCallback),
        elementCallback(elementCallback)
    {
    }
};

}}
#endif // BUILDERS_BUILDERCONTEXT_HPP_DEFINED
