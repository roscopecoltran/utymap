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

/// Geometry line in linear form. General form:
/// Ax + By + C = 0;
/// <see href="http://en.wikipedia.org/wiki/Linear_equation"/>
struct LineLinear
{
    double A;
    double B;
    double C;

    /// Linear line from two points on line.
    LineLinear(const Point& p1, const Point& p2)
    {
        A = p1.y - p2.y;
        B = p2.x - p1.x;
        C = p1.x * p2.y - p2.x * p1.y;
    }

    // Linear line represented by coefficients.
    LineLinear(double pA, double pB, double pC)
    {
        A = pA;
        B = pB;
        C = pC;
    }

    /// Gets collision point of two lines.
    inline Point collide(const LineLinear& line)
    {
        return collide(*this, line);
    }

    /// <summary> Collision point of two lines. </summary>
    inline static Point collide(const LineLinear& line1, const LineLinear& line2)
    {
        return collide(line1.A, line1.B, line1.C, line2.A, line2.B, line2.C);
    }

    /// <summary> Collision point of two lines. </summary>
    inline static Point collide(double A1, double B1, double C1, double A2, double B2, double C2)
    {
        double WAB = A1*B2 - A2*B1;
        double WBC = B1*C2 - B2*C1;
        double WCA = C1*A2 - C2*A1;

        return WAB == 0 ? Point() : Point(WBC / WAB, WCA / WAB);
    }

    /// <summary> Check whether point belongs to line. </summary>
    inline bool contains(const Point& point) const
    {
        return std::abs((point.x * A + point.y * B + C)) < 1E-8;
    }
};

// Represents axis aligned rectangle in 2D space.
struct Rectangle
{
    double xMin, xMax, yMin, yMax;

    LineLinear _left, _right, _bottom, _top;

    Rectangle() : Rectangle(0, 0, 0, 0)
    {
    }

    Rectangle(double xMin, double yMin, double xMax, double yMax) :
        xMin(xMin), yMin(yMin), xMax(xMax), yMax(yMax),
        _left(Point(xMin, yMin), Point(xMin, yMax)),
        _right(Point(xMax, yMin), Point(xMax, yMax)),
        _bottom(Point(xMin, yMin), Point(xMax, yMin)),
        _top(Point(xMin, yMax), Point(xMax, yMax))
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

    // Checks whether point is on border of rectangle.
    inline bool isOnBorder(const Point& point) const
    {
        return _left.contains(point) || _right.contains(point) ||
               _bottom.contains(point) || _top.contains(point);
    }
};

// Represents mesh which uses only primitive types to store data due to interoperability.
struct Mesh
{
    std::string name;
    std::vector<double> vertices;
    std::vector<int> triangles;
    std::vector<int> colors;

    Mesh(const std::string& name) : name(name) { }

    // disable copying to prevent accidential copy
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
};

using Contour = std::vector<utymap::meshing::Point>;

}}

#endif // MESHING_MESHTYPES_HPP_DEFINED
