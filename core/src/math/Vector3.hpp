#ifndef MATH_VECTOR3_HPP_DEFINED
#define MATH_VECTOR3_HPP_DEFINED

namespace utymap { namespace math {

/// Represents vector3 which can be used as point or direction in 3d space
struct Vector3
{
    double x;
    double y;
    double z;
    Vector3() : x(0), y(0), z(0)  { }
    Vector3(double x, double y, double z) : x(x), y(y), z(z) { }

    Vector3 operator*(double scale) const
    {
        return Vector3(x * scale, y *scale, z * scale);
    }

    Vector3 operator/(double scale) const
    {
        return Vector3(x / scale, y / scale, z / scale);
    }

    Vector3 operator+(const Vector3& rhs) const
    {
        return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
    }

    Vector3 operator-(const Vector3& rhs) const
    {
        return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
    }

    double magnitude() const
    {
        return ::std::sqrt(x * x + y * y + z * z);
    }

    Vector3 normalized() const
    {
        double length = magnitude();
        return length > std::numeric_limits<double>::epsilon()
            ? Vector3(x / length, y / length, z / length)
            : Vector3(); // degenerative case
    }

    static Vector3 cross(const Vector3& a, const Vector3& b)
    {
        return Vector3(a.y * b.z - a.z * b.y,
                       a.z * b.x - a.x * b.z,
                       a.x * b.y - a.y * b.x);
    }
};

}}
#endif //MATH_VECTOR3_HPP_DEFINED
