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

    TileLoaderImpl(GeoStore& geoStore, const StyleSheet& stylesheet, StringTable& stringTable, ElevationProvider<double>& eleProvider) :
        geoStore_(geoStore),
        eleProvider_(eleProvider),
        stringTable_(stringTable)
    {
        createBackgroundProperties(stylesheet);
    }

    void loadTile(const QuadKey& quadKey, const std::function<void(Mesh<double>&)>& meshFunc, const std::function<void(Element&)>& elementFunc)
    {
        TerraBuilder terraBuilder(eleProvider_);
        terraBuilder.setBackgroundProperties(backgroundPropertiesMap_[quadKey.levelOfDetail]);

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

    void createBackgroundProperties(const StyleSheet& stylesheet)
    {
        // TODO use StyleProvider instead
        for (const auto& rule : stylesheet.rules) {
            // NOTE expecting that canvas is always first in list of selectors
            const auto& selector = rule.selectors[0];
            if (selector.name != "canvas") continue;
            MeshRegion::Properties properties;
            properties.eleNoiseFreq = std::stof(getDeclaration(rule, BackgroundEleNoiseFreq).value);
            properties.colorNoiseFreq = std::stof(getDeclaration(rule, BackgroundColorNoiseFreq).value);

            for (int lod = selector.zoom.start; lod <= selector.zoom.end; ++lod) {
                backgroundPropertiesMap_[lod] = properties;
            }
        }
    }

    inline Declaration getDeclaration(const Rule& rule, const std::string& key) const
    {
        for (const auto& declaration : rule.declarations) {
            if (declaration.key == key)
                return declaration;
        }
        throw utymap::MapCssException("Cannot find declaration:" + key + " in rule:" 
            + utymap::utils::toString(rule));
    }

    GeoStore& geoStore_;
    StringTable& stringTable_;
    ElevationProvider<double>& eleProvider_;
    std::unordered_map<int, MeshRegion::Properties> backgroundPropertiesMap_;
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