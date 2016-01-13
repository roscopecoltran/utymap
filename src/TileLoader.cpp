#include "Exceptions.hpp"
#include "TileLoader.hpp"
#include "index/GeoUtils.hpp"
#include "terrain/TerraBuilder.hpp"
#include "utils/CompatibilityUtils.hpp"
#include "utils/CoreUtils.hpp"

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
    // Stylesheet constants
    // TODO: use "constexpr auto" instead in future after compiler upgrade
    const std::string BackgroundColorNoiseFreq = "background-color-noise-freq";
    const std::string BackgroundEleNoiseFreq = "background-ele-noise-freq";

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

    TileLoaderImpl(GeoStore& geoStore, const StyleProvider& styleProvider, StringTable& stringTable, ElevationProvider<double>& eleProvider) :
        geoStore_(geoStore),
        styleProvider_(styleProvider),
        eleProvider_(eleProvider),
        stringTable_(stringTable)
    {
        createBackgroundProperties();
    }

    void loadTile(const QuadKey& quadKey, const std::function<void(Mesh<double>&)>& meshFunc, const std::function<void(Element&)>& elementFunc)
    {
        TerraBuilder terraBuilder(eleProvider_);
        terraBuilder.setBackgroundProperties(backgroundPropertiesMap_[quadKey.levelOfDetail]);

        GeoStoreElementVisitor elementVisitor(terraBuilder);
        geoStore_.search(quadKey, elementVisitor, styleProvider_);

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

    void createBackgroundProperties()
    {
        for (int lod = GeoUtils::MinLevelOfDetails; lod <= GeoUtils::MaxLevelOfDetails; ++lod) {
            Style style = styleProvider_.forCanvas(lod);
            MeshRegion::Properties properties;
            properties.eleNoiseFreq = std::stof(getStyleValue(BackgroundEleNoiseFreq, style));
            properties.colorNoiseFreq = std::stof(getStyleValue(BackgroundColorNoiseFreq, style));
            backgroundPropertiesMap_[lod] = properties;

        }
    }

    inline std::string getStyleValue(const std::string& key, const Style& style) 
    {
        uint32_t keyId = stringTable_.getId(key);
        uint32_t valueId = style.get(keyId);
        return std::move(stringTable_.getString(valueId));
    }

    GeoStore& geoStore_;
    StringTable& stringTable_;
    const StyleProvider& styleProvider_;
    ElevationProvider<double>& eleProvider_;
    std::unordered_map<int, MeshRegion::Properties> backgroundPropertiesMap_;
};


void TileLoader::loadTile(const QuadKey& quadKey, const std::function<void(Mesh<double>&)>& meshFunc, const std::function<void(Element&)>& elementFunc)
{
    pimpl_->loadTile(quadKey, meshFunc, elementFunc);
}

TileLoader::TileLoader(GeoStore& geoStore, const StyleProvider& styleProvider, StringTable& stringTable, ElevationProvider<double>& eleProvider) :
    pimpl_(std::unique_ptr<TileLoader::TileLoaderImpl>(new TileLoader::TileLoaderImpl(geoStore, styleProvider, stringTable, eleProvider)))
{
}

TileLoader::~TileLoader()
{
}