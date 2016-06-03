#ifndef UTILS_GEOMETRYUTILS_HPP_DEFINED
#define UTILS_GEOMETRYUTILS_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "meshing/MeshTypes.hpp"
#include "meshing/Polygon.hpp"

#include <algorithm>
#include <vector>

namespace utymap { namespace utils {

    // Checks whether geocoordinates are in clockwise oreder
    template <typename T>
    inline bool isClockwise(const T& coordinates)
    {
        auto size = coordinates.size();
        double area = 0.0;
        for (std::size_t p = size - 1, q = 0; q < size; p = q++)
            area += coordinates[p].longitude*coordinates[q].latitude - coordinates[q].longitude*coordinates[p].latitude;
        return area < 0;
    }

    // Gets circle parameters from geocoordinates
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

    // Gets circle parameters from rectangle.
    inline void getCircle(const utymap::meshing::Rectangle& rectangle, utymap::meshing::Vector2& center, double& radius)
    {
        double radiusX = (rectangle.xMax - rectangle.xMin) / 2;
        double radiusY = (rectangle.yMax - rectangle.yMin) / 2;

        double centerX = rectangle.xMin + radiusX;
        double centerY = rectangle.yMin + radiusY;

        radius = radiusY;
        center = utymap::meshing::Vector2(centerX, centerY);
    }

    // Gets centroid.
    inline utymap::meshing::Vector2 getCentroid(const utymap::meshing::Polygon& polygon)
    {
        double centroidX = 0.0;
        double centroidY = 0.0;

        for (std::size_t i = 0; i < polygon.points.size(); i+=2) {
            centroidX += polygon.points[i];
            centroidY += polygon.points[i+1];
        }

        auto count = polygon.points.size() / 2;
        centroidX /= count;
        centroidY /= count;

        return utymap::meshing::Vector2(centroidX, centroidY);
    }

    // Iterates through polygon outers and call visitor with rectangle of this outer
    template <typename Visitor>
    inline void outerRectangles(const utymap::meshing::Polygon& polygon, Visitor& visitor)
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
}}

#endif // UTILS_GEOMETRYUTILS_HPP_DEFINED
