#ifndef HEIGHTMAP_ELEVATIONPROVIDER_HPP_DEFINED
#define HEIGHTMAP_ELEVATIONPROVIDER_HPP_DEFINED

namespace utymap { namespace heightmap {

// Provides the way to get elevation for given location.
template <typename T>
class ElevationProvider
{
public:
    virtual T getElevation(T x, T y) = 0;
};

}}

#endif // HEIGHTMAP_ELEVATIONPROVIDER_HPP_DEFINED
