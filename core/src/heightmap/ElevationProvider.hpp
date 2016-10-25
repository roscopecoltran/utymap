#ifndef HEIGHTMAP_ELEVATIONPROVIDER_HPP_DEFINED
#define HEIGHTMAP_ELEVATIONPROVIDER_HPP_DEFINED

#include "BoundingBox.hpp"
#include "GeoCoordinate.hpp"
#include "QuadKey.hpp"

namespace utymap { namespace heightmap {

/// Provides the way to get elevation for given location.
class ElevationProvider
{
public:
    /// Preloads data for given bounding box.
    virtual void preload(const utymap::BoundingBox&) = 0;

    /// Preloads data for given quadkey.
    virtual void preload(const utymap::QuadKey&) = 0;

    /// Gets elevation for given geocoordinate.
    virtual double getElevation(const QuadKey& quadkey, const utymap::GeoCoordinate&) const = 0;

    /// Gets elevation for given geocoordinate.
    virtual double getElevation(const QuadKey& quadkey, double latitude, double longitude) const = 0;

    virtual ~ElevationProvider() = default;
};

}}

#endif // HEIGHTMAP_ELEVATIONPROVIDER_HPP_DEFINED
