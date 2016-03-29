#ifndef UTILS_MATHUTILS_HPP_DEFINED
#define UTILS_MATHUTILS_HPP_DEFINED

#include <algorithm>

#ifdef M_PI
static double pi = M_PI;
#else
static double pi = std::acos(-1);
#endif

namespace utymap { namespace utils {

    // clamps value in range
    inline static double clamp(double n, double minValue, double maxValue)
    {
        return std::max(minValue, std::min(n, maxValue));
    }

    // converts degrees to radians.
    inline static double deg2Rad(double degrees)
    {
        return pi*degrees / 180.0;
    }

    // converts radians to degrees
    inline static double rad2Deg(double radians)
    {
        return 180.0*radians / pi;
    }
}}

#endif // UTILS_MATHUTILS_HPP_DEFINED
