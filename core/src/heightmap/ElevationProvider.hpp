#ifndef HEIGHTMAP_ELEVATIONPROVIDER_HPP_DEFINED
#define HEIGHTMAP_ELEVATIONPROVIDER_HPP_DEFINED

#include "BoundingBox.hpp"
#include "GeoCoordinate.hpp"

namespace utymap { namespace heightmap {

// Provides the way to get elevation for given location.
class ElevationProvider
{
public:
    // Preloads data for given bounding box.
    virtual void preload(const utymap::BoundingBox&) = 0;

    // Gets elevation for given geocoordinate.
    virtual double getElevation(const utymap::GeoCoordinate&) const = 0;

    // Gets elevation for given geocoordinate.
    virtual double getElevation(double latitude, double longitude) const = 0;

    virtual ~ElevationProvider() {}
};

}}

#endif // HEIGHTMAP_ELEVATIONPROVIDER_HPP_DEFINED
