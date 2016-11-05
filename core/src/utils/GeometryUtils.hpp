#ifndef UTILS_GEOMETRYUTILS_HPP_DEFINED
#define UTILS_GEOMETRYUTILS_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "meshing/MeshTypes.hpp"
#include "meshing/Polygon.hpp"

#include <algorithm>
#include <vector>

namespace utymap { namespace utils {

/// Gets area of polygon
template <typename T>
double getArea(const T& coordinates)
{
    auto size = coordinates.size();
    double area = 0.0;
    for (std::size_t p = size - 1, q = 0; q < size; p = q++)
        area += coordinates[p].longitude*coordinates[q].latitude - coordinates[q].longitude*coordinates[p].latitude;
    return area;
}

/// Checks whether geocoordinates are in clockwise oreder
template <typename T>
bool isClockwise(const T& coordinates)
{
    return getArea(coordinates) < 0;
}

/// Gets circle parameters from geocoordinates
inline void getCircle(const std::vector<utymap::GeoCoordinate>& coordinates, utymap::GeoCoordinate& center, double& radius) 
{
    auto minMaxLong = std::minmax_element(coordinates.begin(), coordinates.end(),
        [](const utymap::GeoCoordinate& left, const utymap::GeoCoordinate& right) {
            return left.longitude < right.longitude;
    });

    auto minMaxLat = std::minmax_element(coordinates.begin(), coordinates.end(),
        [](const utymap::GeoCoordinate& left, const utymap::GeoCoordinate& right) {
            return left.latitude < right.latitude;
    });

    double centerLon = (minMaxLong.first->longitude + (minMaxLong.second->longitude - minMaxLong.first->longitude) / 2);
    double centerLat = (minMaxLat.first->latitude + (minMaxLat.second->latitude - minMaxLat.first->latitude) / 2);

    radius = (minMaxLat.second->latitude - minMaxLat.first->latitude) / 2;
    center = utymap::GeoCoordinate(centerLat, centerLon);
}

/// Gets circle parameters from rectangle.
inline void getCircle(const utymap::meshing::Rectangle& rectangle, utymap::meshing::Vector2& center, utymap::meshing::Vector2& size)
{
    double radiusX = (rectangle.xMax - rectangle.xMin) / 2;
    double radiusY = (rectangle.yMax - rectangle.yMin) / 2;

    double centerX = rectangle.xMin + radiusX;
    double centerY = rectangle.yMin + radiusY;

    size = utymap::meshing::Vector2(radiusX, radiusY);
    center = utymap::meshing::Vector2(centerX, centerY);
}

/// Gets centroid.
inline utymap::meshing::Vector2 getCentroid(const utymap::meshing::Polygon& polygon, const utymap::meshing::Polygon::Range& range)
{
    double centroidX = 0.0;
    double centroidY = 0.0;

    for (std::size_t i = range.first; i < range.second; i += 2) {
        centroidX += polygon.points[i];
        centroidY += polygon.points[i+1];
    }

    auto count = (range.second - range.first) / 2.;
    centroidX /= count;
    centroidY /= count;

    return utymap::meshing::Vector2(centroidX, centroidY);
}

/// Iterates through polygon outers and call visitor with rectangle of this outer
template <typename Visitor>
void outerRectangles(const utymap::meshing::Polygon& polygon, const Visitor& visitor)
{
    if (polygon.outers.size() == 1) {
        visitor(polygon.rectangle);
        return;
    }

    for (const auto& outer : polygon.outers) {
        auto rectangle = utymap::meshing::Rectangle();
        for (auto i = outer.first; i < outer.second; i += 2) {
            rectangle.expand(utymap::meshing::Vector2(polygon.points[i], polygon.points[i + 1]));
        }
        visitor(rectangle);
    }
}

/// Gets intersection point between two segments. If they don't intersect double lowest is returned.
inline double getIntersection(const utymap::meshing::Vector2& start1, const utymap::meshing::Vector2& end1,
                              const utymap::meshing::Vector2& start2, const utymap::meshing::Vector2& end2)
{
    double denom = ((end1.x - start1.x) * (end2.y - start2.y)) - ((end1.y - start1.y) * (end2.x - start2.x));

    //  AB & CD are parallel 
    if (denom == 0)
        return std::numeric_limits<double>::lowest();

    double numer = ((start1.y - start2.y) * (end2.x - start2.x)) - ((start1.x - start2.x) * (end2.y - start2.y));

    double r = numer / denom;

    double numer2 = ((start1.y - start2.y) * (end1.x - start1.x)) - ((start1.x - start2.x) * (end1.y - start1.y));

    double s = numer2 / denom;

    return (r < 0 || r > 1) || (s < 0 || s > 1)
        ? std::numeric_limits<double>::lowest()
        : r;
}

/// Gets point along the line. r is computed by getIntersection
inline utymap::meshing::Vector2 getPointAlongLine(const utymap::meshing::Vector2& start, const utymap::meshing::Vector2& end, double r)
{
    return utymap::meshing::Vector2(start.x + (r * (end.x - start.x)), start.y + (r * (end.y - start.y)));
}

}}

#endif // UTILS_GEOMETRYUTILS_HPP_DEFINED
