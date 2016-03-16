#ifndef GEOCOORDINATE_HPP_DEFINED
#define GEOCOORDINATE_HPP_DEFINED

#include <cmath>
#include <limits>

namespace utymap {

struct GeoCoordinate
{
    // Latitude.
    double latitude;
    // Longitude.
    double longitude;

    // TODO specify invalid coordinates instead of (0, 0)
    GeoCoordinate(): latitude(0), longitude(0)
    {
    }

    GeoCoordinate(double latitude, double longitude) :
        latitude(latitude),
        longitude(longitude)
    {
    }

    bool operator==(const GeoCoordinate& rhs) const
    {
        return std::abs(latitude - rhs.latitude) < std::numeric_limits<double>::epsilon() &&
               std::abs(longitude - rhs.longitude) < std::numeric_limits<double>::epsilon();
    }

};

}

#endif // GEOCOORDINATE_HPP_DEFINED
