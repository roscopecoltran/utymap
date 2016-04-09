#ifndef HEIGHTMAP_ELEVATIONPROVIDERFACTORY_HPP_DEFINED
#define HEIGHTMAP_ELEVATIONPROVIDERFACTORY_HPP_DEFINED

#include "QuadKey.hpp"
#include "heightmap/ElevationProvider.hpp"
#include "memory"

namespace utymap { namespace heightmap {

// Provides the way to get specific elevation provider for given quadkey.
class ElevationProviderFactory
{
public:
    ElevationProviderFactory(int flatLodEnd)
    {
    }

    // preload elevation in advance to prevent multithreading issues.
    void preload(const utymap::QuadKey& quadKey) 
    {

    }

    // gets ElevationProvider for given quad key.
    std::shared_ptr<ElevationProvider> get(const utymap::QuadKey& quadKey)
    {
        // TODO
    }
};

}}

#endif // HEIGHTMAP_ELEVATIONPROVIDERFACTORY_HPP_DEFINED
