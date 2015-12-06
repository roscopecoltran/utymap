#ifndef MESHING_MESHTYPES_HPP_DEFINED
#define MESHING_MESHTYPES_HPP_DEFINED

#include <algorithm>
#include <cstdint>
#include <vector>

namespace utymap { namespace meshing {

// represents point in 2D space.
template <typename T>
struct Point
{
    T x;
    T y;
    Point() : x(0), y(0)  { }
    Point(T x, T y) : x(x), y(y) { }
};

// Represents axis aligned rectangle in 2D space.
template <typename T>
struct Rectangle
{
    T xMin, xMax, yMin, yMax;

    void expand(const std::vector<Point<T>>& contour)
    {
        for (const Point<T>& p : contour) {
            xMin = std::min(xMin, p.x);
            yMin = std::min(yMin, p.y);
            xMax = std::max(xMax, p.x);
            yMax = std::max(yMax, p.y);
        }
    }

    inline bool contains(Point<T> pt)
    {
        return ((pt.x >= xMin) && (pt.x <= xMax) && (pt.y >= yMin) && (pt.y <= yMax));
    }
};

// Represents mesh. It uses only primitive types due to interoperability.
template <typename T>
struct Mesh
{
    std::vector<double> vertices;
    std::vector<int> triangles;
};

}}

#endif // MESHING_MESHTYPES_HPP_DEFINED
