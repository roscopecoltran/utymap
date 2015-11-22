#include "BoundingBox.hpp"

using namespace utymap;

BoundingBox::BoundingBox(const GeoCoordinate& minPoint, const GeoCoordinate& maxPoint) :
    minPoint(minPoint), maxPoint(maxPoint)
{
}

void BoundingBox::expand(const BoundingBox& rhs)
{
    minPoint.latitude = minPoint.latitude < rhs.minPoint.latitude ? minPoint.latitude : rhs.minPoint.latitude;
    minPoint.longitude = minPoint.longitude < rhs.minPoint.longitude ? minPoint.longitude : rhs.minPoint.longitude;

    maxPoint.latitude = maxPoint.latitude > rhs.maxPoint.latitude ? maxPoint.latitude : rhs.maxPoint.latitude;
    maxPoint.longitude = maxPoint.longitude > rhs.maxPoint.longitude ? maxPoint.longitude : rhs.maxPoint.longitude;

}

bool BoundingBox::contains(const GeoCoordinate& coordinate) const
{
    return coordinate.latitude > minPoint.latitude && coordinate.longitude > minPoint.longitude &&
           coordinate.latitude < maxPoint.latitude && coordinate.longitude < maxPoint.longitude;
}

bool BoundingBox::contains(const BoundingBox& bbox) const
{
    return contains(bbox.minPoint) && contains(bbox.maxPoint);
}

bool BoundingBox::intersects(const BoundingBox& rhs) const
{
    double minX = minPoint.latitude < rhs.minPoint.latitude ? rhs.minPoint.latitude : minPoint.latitude;
    double minY = minPoint.longitude < rhs.minPoint.longitude ? rhs.minPoint.longitude : minPoint.longitude;
    double maxX = maxPoint.latitude > rhs.maxPoint.latitude ? rhs.maxPoint.latitude : maxPoint.latitude;
    double maxY = maxPoint.longitude > rhs.maxPoint.longitude ? rhs.maxPoint.longitude : maxPoint.longitude;

    return minX < maxX && minY < maxY;
}
