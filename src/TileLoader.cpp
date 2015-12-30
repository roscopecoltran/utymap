#include "TileLoader.hpp"
#include "index/GeoUtils.hpp"
#include "terrain/TerraBuilder.hpp"

#include <unordered_map>

using namespace utymap;
using namespace utymap::heightmap;
using namespace utymap::entities;
using namespace utymap::terrain;
using namespace utymap::index;
using namespace utymap::mapcss;
using namespace utymap::meshing;

class TileLoader::TileLoaderImpl
{
private:

class GeoStoreElementVisitor : public ElementVisitor
{
public:
    GeoStoreElementVisitor(TerraBuilder& terraBuilder):
        terraBuilder_(terraBuilder)
    {
    }

    void visitNode(const Node&)
    {
    }

    void visitWay(const Way&)
    {
    }

    void visitArea(const Area&)
    {
    }

    void visitRelation(const Relation&)
    {
    }
private:
    TerraBuilder& terraBuilder_;
};

public:

    TileLoaderImpl(GeoStore& geoStore, const StyleSheet& stylesheet, StringTable& stringTable, ElevationProvider<double>& eleProvider) :
        geoStore_(geoStore),
        eleProvider_(eleProvider),
        stringTable_(stringTable)
    {
        createBackgroundMeshRegions(stylesheet);
    }

    void loadTile(const QuadKey& quadKey, const std::function<void(Mesh<double>&)>& meshFunc, const std::function<void(Element&)>& elementFunc)
    {
        TerraBuilder terraBuilder(eleProvider_);

        // TODO
        //terraBuilder.setBackgroundProperties(MeshRegion)

        GeoStoreElementVisitor elementVisitor(terraBuilder);
        geoStore_.search(quadKey, elementVisitor);

        Rectangle<double> tileRect = createRectFromQuadKey(quadKey);
        Mesh<double> mesh = terraBuilder.build(tileRect, quadKey.levelOfDetail);
        meshFunc(mesh);
    }

private:

    Rectangle<double> createRectFromQuadKey(const QuadKey& quadKey)
    {
        BoundingBox bbox = GeoUtils::quadKeyToBoundingBox(quadKey);
        return Rectangle<double>(
            bbox.minPoint.longitude,
            bbox.minPoint.latitude,
            bbox.maxPoint.longitude,
            bbox.maxPoint.latitude
        );
    }

    void createBackgroundMeshRegions(const StyleSheet& stylesheet)
    {
        for (auto& rule : stylesheet.rules) {
            // NOTE expecting that canvas is always first in list
            auto& selector = rule.selectors[0];
            if (selector.name != "canvas") continue;

        }
    }

    GeoStore& geoStore_;
    StringTable& stringTable_;
    ElevationProvider<double>& eleProvider_;
};


void TileLoader::loadTile(const QuadKey& quadKey, const std::function<void(Mesh<double>&)>& meshFunc, const std::function<void(Element&)>& elementFunc)
{
    pimpl_->loadTile(quadKey, meshFunc, elementFunc);
}

TileLoader::TileLoader(GeoStore& geoStore, const StyleSheet& stylesheet, StringTable& stringTable, ElevationProvider<double>& eleProvider) :
pimpl_(std::unique_ptr<TileLoader::TileLoaderImpl>(new TileLoader::TileLoaderImpl(geoStore, stylesheet, stringTable, eleProvider)))
{
}

TileLoader::~TileLoader()
{
}