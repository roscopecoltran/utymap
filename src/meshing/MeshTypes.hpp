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

    inline void expand(const std::vector<Point<T>>& contour)
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

// Represents point in 3D space.
template <typename T>
struct Vertex
{
    T x;
    T y;
    T z;

    Vertex() : x(0), y(0), z(0) { }
    Vertex(T x, T y, T z) : x(x), y(y), z(z) { }
};

// Represents triangle which stores vertex indicies.
struct Triangle
{
    std::uint16_t v0;
    std::uint16_t v1;
    std::uint16_t v2;

    Triangle() : v0(0), v1(0), v2(0) { }
    Triangle(std::uint16_t v0, std::uint16_t v1, std::uint16_t v2) 
        : v0(v0), v1(v1), v2(v2) { }
};

// Represents mesh in 3D space.
template <typename T>
struct Mesh
{
    std::vector<Vertex<T>> vertices;
    std::vector<Triangle> triangles;
};

}}

#endif // MESHING_MESHTYPES_HPP_DEFINED
