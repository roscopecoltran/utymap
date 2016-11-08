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

/// Creates rotation matrix for rotation around given vector in 3D space.
inline std::function<utymap::math::Vector3(const utymap::math::Vector3&)> createRotationFunc(const utymap::math::Vector3& n, double angle)
{
    double cos = std::cos(angle);
    double sin = std::sin(angle);

    double m11 = n.x * n.x * (1 - cos) + cos;
    double m21 = n.x * n.y * (1 - cos) - n.z * sin;
    double m31 = n.x * n.z * (1 - cos) + n.y * sin;

    double m12 = n.x * n.y * (1 - cos) + n.z * sin;
    double m22 = n.y * n.y * (1 - cos) + cos;
    double m32 = n.y * n.z * (1 - cos) - n.x * sin;

    double m13 = n.x * n.z * (1 - cos) - n.y * sin;
    double m23 = n.y * n.z * (1 - cos) + n.x * sin;
    double m33 = n.z * n.z * (1 - cos) + cos;

    return[=](const utymap::math::Vector3& v) {
        return utymap::math::Vector3(v.x*m11 + v.y*m21 + v.z*m31,
                                     v.x*m12 + v.y*m22 + v.z*m32,
                                     v.x*m13 + v.y*m23 + v.z*m33);
    };
}

}}

#endif // UTILS_MATHUTILS_HPP_DEFINED
