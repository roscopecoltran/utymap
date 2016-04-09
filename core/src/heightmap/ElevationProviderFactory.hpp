#ifndef HEIGHTMAP_ELEVATIONPROVIDERFACTORY_HPP_DEFINED
#define HEIGHTMAP_ELEVATIONPROVIDERFACTORY_HPP_DEFINED

#include "QuadKey.hpp"
#include "heightmap/ElevationProvider.hpp"
#include "heightmap/FlatElevationProvider.hpp"
#include "heightmap/SrtmElevationProvider.hpp"
#include "utils/GeoUtils.hpp"

#include "memory"

namespace utymap { namespace heightmap {

// Provides the way to get specific elevation provider for given quadkey.
class ElevationProviderFactory
{
public:
    ElevationProviderFactory(const std::string& srtmDataPath, int flatLodEnd) : 
        flatLodEnd_(flatLodEnd),
        srtmEleProvider_(new SrtmElevationProvider(srtmDataPath)),
        flatEleProvider_(new FlatElevationProvider())
    {
    }

    // Preloads elevation in advance to prevent multithreading issues.
    void preload(const utymap::QuadKey& quadKey) 
    {
        auto boundingBox = utymap::utils::GeoUtils::quadKeyToBoundingBox(quadKey);
        get(quadKey)->preload(boundingBox);
    }

    // Gets ElevationProvider for given quadkey.
    std::shared_ptr<ElevationProvider> get(const utymap::QuadKey& quadKey) const
    {
        return quadKey.levelOfDetail <= flatLodEnd_ ? flatEleProvider_ : srtmEleProvider_;
    }

private:
    int flatLodEnd_;
    std::shared_ptr<FlatElevationProvider> flatEleProvider_;
    std::shared_ptr<SrtmElevationProvider> srtmEleProvider_;
};

}}

#endif // HEIGHTMAP_ELEVATIONPROVIDERFACTORY_HPP_DEFINED
