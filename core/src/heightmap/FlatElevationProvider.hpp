#ifndef HEIGHTMAP_FLATELEVATIONPROVIDER_HPP_DEFINED
#define HEIGHTMAP_FLATELEVATIONPROVIDER_HPP_DEFINED

#include "heightmap/ElevationProvider.hpp"

namespace utymap { namespace heightmap {

// Simple implementation of ElevationProvider which returns zero for all places.
class FlatElevationProvider: public ElevationProvider
{
public:

    void preload(const utymap::BoundingBox&) {}

    double getElevation(const utymap::GeoCoordinate&) const { return 0; }

    double getElevation(double, double) const { return 0; };
};

}}

#endif // HEIGHTMAP_FLATELEVATIONPROVIDER_HPP_DEFINED
