#ifndef TILELOADER_HPP_DEFINED
#define TILELOADER_HPP_DEFINED

#include "entities/Element.hpp"
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

    TileLoader();
    ~TileLoader();

    // Configures tile loader.
    void configure(const std::string& configPath);

    // Loads tile for given quadkey.
    void loadTile(const utymap::QuadKey& quadKey,
                  std::function<utymap::meshing::Mesh<double>>& meshFunc,
                  std::function<utymap::entities::Element>& elementFunc);

private:
    class TileLoaderImpl;
    std::unique_ptr<TileLoaderImpl> pimpl_;
};

}
#endif // TILELOADER_HPP_DEFINED
