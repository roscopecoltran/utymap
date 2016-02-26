#ifndef UTILS_NOISEUTILS_HPP_DEFINED
#define UTILS_NOISEUTILS_HPP_DEFINED

namespace utymap { namespace utils {

// Provides noise generation functions.
// Ported from C# code from here : http ://catlikecoding.com/unity/tutorials/noise/
class NoiseUtils
{
private:
    struct Vector3
    {
        double x, y, z;

        Vector3(double x, double y, double z) 
            : x(x), y(y), z(z)
        {
        }
    };

public:

    // Calculates perlin 3D noise in [-1;1] range.
    static double perlin3D(double x, double y, double z, double freq);

private:

    static inline double dot(const Vector3& g, double x, double y, double z)
    {
        return g.x*x + g.y*y + g.z*z;
    }

    static inline double smooth(double t)
    {
        return t*t*t*(t*(t * 6 - 15) + 10);
    }

    static const int HashMask = 255;
    static const int GradientsMask3D = 15;
    static const Vector3 Gradients3D[];
    static const int Hash[];
};
}}

#endif // UTILS_NOISEUTILS_HPP_DEFINED