#ifndef TILELOADER_HPP_DEFINED
#define TILELOADER_HPP_DEFINED

#include "QuadKey.hpp"
#include "builders/ElementBuilder.hpp"
#include "entities/Element.hpp"
#include "index/GeoStore.hpp"
#include "mapcss/StyleProvider.hpp"
#include "meshing/MeshTypes.hpp"

#include <functional>
#include <string>
#include <memory>

namespace utymap {

// Responsible for loading tile.
class TileLoader
{
public:

    typedef std::function<void(const utymap::meshing::Mesh<double>&)> MeshCallback;
    typedef std::function<void(const utymap::entities::Element&)> ElementCallback;
    typedef std::function<std::shared_ptr<utymap::builders::ElementBuilder>(const MeshCallback&, const ElementCallback&)> ElementBuilderFactory;

    TileLoader(utymap::index::GeoStore& geoStore,
               utymap::index::StringTable& stringTable,
               const utymap::mapcss::StyleProvider& styleProvider);

    ~TileLoader();

    // Registers factory method for element builder.
    void registerElementBuilder(const std::string& name, ElementBuilderFactory factory);

    // Loads tile for given quadkey.
    // TODO rename to build() and class: TileBuilder
    void loadTile(const utymap::QuadKey& quadKey, MeshCallback meshFunc, ElementCallback elementFunc);


private:
    class TileLoaderImpl;
    std::unique_ptr<TileLoaderImpl> pimpl_;
};

}
#endif // TILELOADER_HPP_DEFINED
