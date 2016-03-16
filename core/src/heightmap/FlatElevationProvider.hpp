#ifndef HEIGHTMAP_FLATELEVATIONPROVIDER_HPP_DEFINED
#define HEIGHTMAP_FLATELEVATIONPROVIDER_HPP_DEFINED

#include "heightmap/FlatElevationProvider.hpp"

namespace utymap { namespace heightmap {

// Provides the way to get elevation for given location.
class FlatElevationProvider: public ElevationProvider
{
public:
    inline double getElevation(double x, double y) { return 0; };
};

}}

#endif // HEIGHTMAP_FLATELEVATIONPROVIDER_HPP_DEFINED
