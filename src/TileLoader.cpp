#include "TileLoader.hpp"

using namespace utymap;
using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::meshing;

class TileLoader::TileLoaderImpl
{
public:

    TileLoaderImpl(GeoStore& geoStore) :
        geoStore_(geoStore)
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
};


void TileLoader::loadTile(const QuadKey& quadKey, const std::function<void(Mesh<double>&)>& meshFunc, const std::function<void(Element&)>& elementFunc)
{
    pimpl_->loadTile(quadKey, meshFunc, elementFunc);
}

TileLoader::TileLoader(GeoStore& geoStore) :
    pimpl_(std::unique_ptr<TileLoader::TileLoaderImpl>(new TileLoader::TileLoaderImpl(geoStore)))
{
}

TileLoader::~TileLoader()
{
}