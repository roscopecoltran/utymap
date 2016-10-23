#ifndef HEIGHTMAP_GRIDELEVATIONPROVIDER_HPP_DEFINED
#define HEIGHTMAP_GRIDELEVATIONPROVIDER_HPP_DEFINED

#include "heightmap/ElevationProvider.hpp"

#include <string>

namespace utymap { namespace heightmap {

// Provides the way to get elevation for given location from grid 
// represended by comma separated list of integers
class GridElevationProvider final : public ElevationProvider
{
public:
    GridElevationProvider(std::string dataDirectory) :
        dataDirectory_(dataDirectory)
    {
    }

    void preload(const utymap::BoundingBox& boundingBox) override
    {
    }

    /// Gets elevation for given geocoordinate.
    double getElevation(const utymap::GeoCoordinate& coordinate) const override
    {
        return 0;
    }

    /// Gets elevation for given geocoordinate.
    double getElevation(double latitude, double longitude) const override
    {

    }
private:
    const std::string dataDirectory_;
};

}}

#endif // HEIGHTMAP_GRIDELEVATIONPROVIDER_HPP_DEFINED
