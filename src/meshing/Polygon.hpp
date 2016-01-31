#ifndef MESHING_POLYGON_HPP_DEFINED
#define MESHING_POLYGON_HPP_DEFINED

#include "meshing/MeshTypes.hpp"

#include <stdexcept>

namespace utymap { namespace meshing {

// Represents polygon in 2D space
template <typename T>
class Polygon
{
public:
    std::vector<T> points;
    std::vector<T> holes;
    std::vector<int> segments;

    Polygon(size_t numberOfPoints, size_t numberOfHoles)
    {
        points.reserve(numberOfPoints * 2);
        holes.reserve(numberOfHoles * 2);
        segments.reserve(numberOfPoints * 2);
    }

    // adds outer poings.
    void addContour(const std::vector<Point<T>>& points)
    {
        addContour(points, false);
    }

    // adds hole
    void addHole(const std::vector<Point<T>>& points)
    {
        addContour(points, true);
    }

private:

    void addContour(const std::vector<Point<T>>& contour, bool isHole)
    {
        if (isHole) {
            Point<T> pointInside;
            if (!findPointInPolygon(contour, pointInside)) {
                // TODO log error
                return;
            }
            holes.push_back(pointInside.x);
            holes.push_back(pointInside.y);
        }

        int count = contour.size();
        if (contour[0] == contour[count - 1])
            count--;

        int offset = segments.size() / 2;
        for (int i = 0; i < count; ++i) {
            Point<T> point = contour[i];
            points.push_back(point.x);
            points.push_back(point.y);
            segments.push_back(offset + i);
            segments.push_back(offset + ((i + 1) % count));
        }
    }

    // tries to find point in polygon.
    bool findPointInPolygon(const std::vector<Point<T>>& contour, Point<T>& point)
    {
        Rectangle<T> bounds;
        bounds.expand(contour);

        int length = contour.size();
        int limit = 8;

        Point<T> a, b; // Current edge.
        double cx, cy; // Center of current edge.
        double dx, dy; // Direction perpendicular to edge.

        if (contour.size() == 3) {
            point = Point<T>((contour[0].x + contour[1].x + contour[2].x) / 3,
                (contour[0].y + contour[1].y + contour[2].y) / 3);
            return true;
        }

        for (int i = 0; i < length; i++) {
            a = contour[i];
            b = contour[(i + 1) % length];

            cx = (a.x + b.x) / 2;
            cy = (a.y + b.y) / 2;

            dx = (b.y - a.y) / 1.374;
            dy = (a.x - b.x) / 1.374;

            for (int j = 1; j <= limit; j++)
            {
                // Search to the right of the segment.
                point.x = cx + dx / j;
                point.y = cy + dy / j;

                if (bounds.contains(point) && isPointInPolygon(point, contour))
                    return true;

                // Search on the other side of the segment.
                point.x = cx - dx / j;
                point.y = cy - dy / j;

                if (bounds.contains(point) && isPointInPolygon(point, contour))
                    return true;
            }
        }

        return false;
    }

    // checks whether point in polygon using ray casting algorithm
    bool isPointInPolygon(Point<T>& point, const std::vector<Point<T>>& poly)
    {
        bool inside = false;

        double x = point.x;
        double y = point.y;

        int count = poly.size();

        for (int i = 0, j = count - 1; i < count; i++) {
            if (((poly[i].y < y && poly[j].y >= y) || (poly[j].y < y && poly[i].y >= y))
                && (poly[i].x <= x || poly[j].x <= x)) {
                inside ^= (poly[i].x + (y - poly[i].y) / (poly[j].y - poly[i].y) * (poly[j].x - poly[i].x) < x);
            }

            j = i;
        }

        return inside;
    }

};


}}

#endif // MESHING_POLYGON_HPP_DEFINED
