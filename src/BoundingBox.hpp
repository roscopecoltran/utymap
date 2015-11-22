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
        minPoint(minPoint),
        maxPoint(maxPoint)
    {
    }

    // Expands bounding box by given.
    void expand(const BoundingBox&);
    // Checks whether given bounding box inside the current one.
    bool contains(const BoundingBox&) const;
    // Checks whether given coordinate inside the bounding box.
    bool contains(const GeoCoordinate&) const;
    // Checks whether given bounding box intersects the current one.
    bool intersects(const BoundingBox&) const;
};

}
#endif // BOUNDINGBOX_HPP_DEFINED
