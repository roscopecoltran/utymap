#ifndef HEIGHTMAP_FLATELEVATIONPROVIDER_HPP_DEFINED
#define HEIGHTMAP_FLATELEVATIONPROVIDER_HPP_DEFINED

#include "heightmap/FlatElevationProvider.hpp"

namespace utymap { namespace heightmap {

// Provides the way to get elevation for given location.
template <typename T>
class FlatElevationProvider: public ElevationProvider<T>
{
public:
    inline T getElevation(T x, T y) { return 0; };
};

}}

#endif // HEIGHTMAP_FLATELEVATIONPROVIDER_HPP_DEFINED
