#ifndef HEIGHTMAP_ELEVATIONPROVIDER_HPP_DEFINED
#define HEIGHTMAP_ELEVATIONPROVIDER_HPP_DEFINED

namespace utymap { namespace heightmap {

// Provides the way to get elevation for given location.
class ElevationProvider
{
public:
    virtual double getElevation(double x, double y) = 0;

    virtual ~ElevationProvider() {}
};

}}

#endif // HEIGHTMAP_ELEVATIONPROVIDER_HPP_DEFINED
