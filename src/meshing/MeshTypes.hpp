#ifndef MESHING_MESHTYPES_HPP_DEFINED
#define MESHING_MESHTYPES_HPP_DEFINED

#include <algorithm>
#include <cstdint>
#include <string>
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

    bool operator==(const Point& rhs) const
    {
        return x == rhs.x && y == rhs.y;
    }

    bool operator!=(const Point& rhs) const
    {
        return this != rhs;
    }
};

// Represents axis aligned rectangle in 2D space.
template <typename T>
struct Rectangle
{
    T xMin, xMax, yMin, yMax;

    Rectangle() :
        xMin(0),
        xMax(0),
        yMin(0),
        yMax(0)
    {
    }

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
    std::string name;
    std::vector<T> vertices;
    std::vector<int> triangles;
    std::vector<int> colors;
};

template<typename T>
using Contour = std::vector<utymap::meshing::Point<T>>;

}}

#endif // MESHING_MESHTYPES_HPP_DEFINED
