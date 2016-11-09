#ifndef UTILS_MATHUTILS_HPP_DEFINED
#define UTILS_MATHUTILS_HPP_DEFINED

#include "math/Vector3.hpp"

#include <algorithm>
#include <array>
#include <functional>

#ifdef M_PI
static double pi = M_PI;
#else
static double pi = std::acos(-1);
#endif

namespace utymap { namespace utils {

/// Clamps value in range
inline double clamp(double n, double minValue, double maxValue)
{
    return std::max(minValue, std::min(n, maxValue));
}

/// converts degrees to radians.
inline double deg2Rad(double degrees)
{
    return pi*degrees / 180.0;
}

/// converts radians to degrees
inline double rad2Deg(double radians)
{
    return 180.0*radians / pi;
}

}}
#endif // UTILS_MATHUTILS_HPP_DEFINED
