#ifndef GEOCOORDINATE_HPP_DEFINED
#define GEOCOORDINATE_HPP_DEFINED

namespace utymap {

struct GeoCoordinate
{
    // Latitude.
    double latitude;
    // Longitude.
    double longitude;

    GeoCoordinate(): latitude(0), longitude(0)
    {
    }

    GeoCoordinate(double latitude, double longitude) :
        latitude(latitude),
        longitude(longitude)
    {
    }
};

}

#endif // GEOCOORDINATE_HPP_DEFINED
