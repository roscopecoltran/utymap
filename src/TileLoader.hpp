#ifndef TILELOADER_HPP_DEFINED
#define TILELOADER_HPP_DEFINED

#include "entities/Element.hpp"
#include "index/GeoStore.hpp"
#include "meshing/MeshTypes.hpp"
#include "QuadKey.hpp"

#include <functional>
#include <string>
#include <memory>

namespace utymap {

// Responsible for loading tile.
class TileLoader
{
public:

    TileLoader(utymap::index::GeoStore& geoStore);
    ~TileLoader();

    // Loads tile for given quadkey.
    void loadTile(const utymap::QuadKey& quadKey,
                  const std::function<void(utymap::meshing::Mesh<double>&)>& meshFunc,
                  const std::function<void(utymap::entities::Element&)>& elementFunc);

private:
    class TileLoaderImpl;
    std::unique_ptr<TileLoaderImpl> pimpl_;
};

}
#endif // TILELOADER_HPP_DEFINED
