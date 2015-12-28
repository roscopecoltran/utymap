#include "TileLoader.hpp"

using namespace utymap;
using namespace utymap::heightmap;
using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::meshing;

class TileLoader::TileLoaderImpl
{
public:

    TileLoaderImpl(GeoStore& geoStore, ElevationProvider<double>& eleProvider) :
        geoStore_(geoStore),
        eleProvider_(eleProvider)
    {
    }

    void loadTile(const QuadKey& quadKey, const std::function<void(Mesh<double>&)>& meshFunc, const std::function<void(Element&)>& elementFunc)
    {
        // TODO read index
        Mesh<double> mesh;
        meshFunc(mesh);

        Element element;
        elementFunc(element);
    }

private:
    GeoStore& geoStore_;
    ElevationProvider<double>& eleProvider_;
};


void TileLoader::loadTile(const QuadKey& quadKey, const std::function<void(Mesh<double>&)>& meshFunc, const std::function<void(Element&)>& elementFunc)
{
    pimpl_->loadTile(quadKey, meshFunc, elementFunc);
}

TileLoader::TileLoader(GeoStore& geoStore, ElevationProvider<double>& eleProvider) :
    pimpl_(std::unique_ptr<TileLoader::TileLoaderImpl>(new TileLoader::TileLoaderImpl(geoStore, eleProvider)))
{
}

TileLoader::~TileLoader()
{
}