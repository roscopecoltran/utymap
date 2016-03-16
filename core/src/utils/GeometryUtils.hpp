#ifndef UTILS_GEOMETRYUTILS_HPP_DEFINED
#define UTILS_GEOMETRYUTILS_HPP_DEFINED

#include "GeoCoordinate.hpp"

#include <vector>

namespace utymap { namespace utils {

    inline bool isClockwise(const std::vector<utymap::GeoCoordinate>& coordinates)
    {
        auto size = coordinates.size();
        double area = 0.0;
        for (int p = size - 1, q = 0; q < size; p = q++)
            area += coordinates[p].longitude*coordinates[q].latitude - coordinates[q].longitude*coordinates[p].latitude;
        return area < 0;
    }
}}

#endif // UTILS_GEOMETRYUTILS_HPP_DEFINED
