#include "TileLoader.hpp"

using namespace utymap;
using namespace utymap::entities;
using namespace utymap::meshing;

class TileLoader::TileLoaderImpl
{
public:

    void configure(const std::string& configPath)
    {
        // TODO load string table and index;
    }

    void loadTile(const QuadKey& quadKey, std::function<void(Mesh<double>&)> meshFunc, std::function<void(Element&)> elementFunc)
    {
    }

private:

};

void TileLoader::configure(const std::string& configPath)
{
    pimpl_->configure(configPath);
}

void TileLoader::loadTile(const QuadKey& quadKey, std::function<void(Mesh<double>&)> meshFunc, std::function<void(Element&)> elementFunc)
{
    pimpl_->loadTile(quadKey, meshFunc, elementFunc);
}

TileLoader::TileLoader() :
    pimpl_(std::unique_ptr<TileLoader::TileLoaderImpl>(new TileLoader::TileLoaderImpl()))
{
}

TileLoader::~TileLoader()
{
}