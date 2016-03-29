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
}}

#endif // UTILS_MATHUTILS_HPP_DEFINED
