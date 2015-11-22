#ifndef BOUNDINGBOX_HPP_DEFINED
#define BOUNDINGBOX_HPP_DEFINED

#include "GeoCoordinate.hpp"

namespace utymap {

// Represents geo bounding box
struct BoundingBox
{
    // Point with minimal latitude and longitude.
    GeoCoordinate minPoint;
    // Point with maximum latitude and longitude.
    GeoCoordinate maxPoint;

    BoundingBox(const GeoCoordinate& minPoint, const GeoCoordinate& maxPoint) :
        minPoint(minPoint), maxPoint(maxPoint) 
    {
    }

    BoundingBox& operator+=(const BoundingBox& rhs)
    {
        expand(rhs);
        return *this;
    }

    // Expands bounding box by given.
    inline void expand(const BoundingBox& rhs)
    {
        minPoint.latitude = minPoint.latitude < rhs.minPoint.latitude ? minPoint.latitude : rhs.minPoint.latitude;
        minPoint.longitude = minPoint.longitude < rhs.minPoint.longitude ? minPoint.longitude : rhs.minPoint.longitude;

        maxPoint.latitude = maxPoint.latitude > rhs.maxPoint.latitude ? maxPoint.latitude : rhs.maxPoint.latitude;
        maxPoint.longitude = maxPoint.longitude > rhs.maxPoint.longitude ? maxPoint.longitude : rhs.maxPoint.longitude;
    }

    // Checks whether given bounding box inside the current one.
    inline bool contains(const BoundingBox& bbox) const
    {
        return contains(bbox.minPoint) && contains(bbox.maxPoint);
    }

    // Checks whether given coordinate inside the bounding box.
    inline bool contains(const GeoCoordinate& coordinate) const
    {
        return coordinate.latitude > minPoint.latitude && coordinate.longitude > minPoint.longitude &&
            coordinate.latitude < maxPoint.latitude && coordinate.longitude < maxPoint.longitude;
    }

    // Checks whether given bounding box intersects the current one.
    inline bool intersects(const BoundingBox& rhs) const
    {
        double minX = minPoint.latitude < rhs.minPoint.latitude ? rhs.minPoint.latitude : minPoint.latitude;
        double minY = minPoint.longitude < rhs.minPoint.longitude ? rhs.minPoint.longitude : minPoint.longitude;
        double maxX = maxPoint.latitude > rhs.maxPoint.latitude ? rhs.maxPoint.latitude : maxPoint.latitude;
        double maxY = maxPoint.longitude > rhs.maxPoint.longitude ? rhs.maxPoint.longitude : maxPoint.longitude;

        return minX < maxX && minY < maxY;
    }
};

}
#endif // BOUNDINGBOX_HPP_DEFINED
