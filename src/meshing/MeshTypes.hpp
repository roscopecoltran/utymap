#ifndef MESHING_MESHTYPES_HPP_DEFINED
#define MESHING_MESHTYPES_HPP_DEFINED

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <limits>
#include <string>
#include <vector>

namespace utymap { namespace meshing {

// represents point in 2D space.
struct Point
{
    double x;
    double y;
    Point() : x(0), y(0)  { }
    Point(double x, double y) : x(x), y(y) { }

    bool operator==(const Point& rhs) const
    {
        return std::fabs(x - rhs.x) < std::numeric_limits<double>::epsilon() &&
               std::fabs(y - rhs.y) < std::numeric_limits<double>::epsilon();
    }

    bool operator!=(const Point& rhs) const
    {
        return !(*this == rhs);
    }
};

// Represents axis aligned rectangle in 2D space.
struct Rectangle
{
    double xMin, xMax, yMin, yMax;

    Rectangle() :
        xMin(0),
        xMax(0),
        yMin(0),
        yMax(0)
    {
    }

    Rectangle(double xMin, double yMin, double xMax, double yMax) :
        xMin(xMin),
        yMin(yMin),
        xMax(xMax),
        yMax(yMax)
    {
    }

    void expand(const std::vector<Point>& contour)
    {
        for (const Point& p : contour) {
            xMin = std::min(xMin, p.x);
            yMin = std::min(yMin, p.y);
            xMax = std::max(xMax, p.x);
            yMax = std::max(yMax, p.y);
        }
    }

    inline bool contains(const Point& pt) const
    {
        return ((pt.x >= xMin) && (pt.x <= xMax) && (pt.y >= yMin) && (pt.y <= yMax));
    }
};

// Represents mesh. It uses only primitive types due to interoperability.
struct Mesh
{
    std::string name;
    std::vector<double> vertices;
    std::vector<int> triangles;
    std::vector<int> colors;
};

using Contour = std::vector<utymap::meshing::Point>;

}}

#endif // MESHING_MESHTYPES_HPP_DEFINED
