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

    BoundingBox(const GeoCoordinate& minPoint, const GeoCoordinate& maxPoint);

    // Expands bounding box by given.
    BoundingBox& BoundingBox::operator +=(const BoundingBox&);

    // Checks whether given bounding box inside the current one.
    inline bool contains(const BoundingBox&) const;

    // Checks whether given coordinate inside the bounding box.
    inline bool contains(const GeoCoordinate&) const;

    // Checks whether given bounding box intersects the current one.
    inline bool intersects(const BoundingBox&) const;
};

}
#endif // BOUNDINGBOX_HPP_DEFINED
