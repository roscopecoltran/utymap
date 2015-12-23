#ifndef TERRAIN_LINEGRIDSPLITTER_HPP_DEFINED
#define TERRAIN_LINEGRIDSPLITTER_HPP_DEFINED

#include "meshing/MeshTypes.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace utymap { namespace terrain {

// Splits line to segments according to axis aligned grid.
template<typename T>
class LineGridSplitter
{
    typedef utymap::meshing::Point<T> PointT;
    typedef std::vector<PointT> Points;

    struct sort_x
    {
        inline bool operator() (const PointT& a, const PointT& b) { return a.x < b.x; }
    };

    struct sort_reverse_x
    {
        inline bool operator() (const PointT& a, const PointT& b) { return a.x > b.x; }
    };

    struct sort_y
    {
        inline bool operator() (const PointT& a, const PointT& b) { return a.y < b.y; }
    };

    struct sort_reverse_y
    {
        inline bool operator() (const PointT& a, const PointT& b) { return a.y > b.y; }
    };

public:

    LineGridSplitter(int cellSize, int roundDigitCount) : 
        cellSize_(cellSize),
        roundDigitCount_(roundDigitCount)
    {
    }

    // Splits line to segments.
    Points split(PointT s, PointT e)
    {
        PointT start(s.x, s.y);
        PointT end(e.x, e.y);

        Points points(2);
        points.push_back(s);

        double slope = (e.y - s.y) / (e.x - s.x);
        if (std::isinf(slope) || std::abs(slope) < std::numeric_limits<double>::epsilon)
            zeroSlope(s, e, points);
        else
            normalCase(start, end, slope, points);

        return filterResults(points);
    }

private:
    void zeroSlope(PointT start, PointT end, Points& points)
    {
        if (std::abs(start.x - end.x) < std::numeric_limits<double>::epsilon)
        {
            bool isBottomTop = start.y < end.y;
            if (!isBottomTop)
            {
                T tmp = start;
                start = end;
                end = tmp;
            }

            int yStart = (int)std::ceil(start.y);
            int yEnd = (int)std::floor(end.y);
            for (int y = yStart; y <= yEnd; y += cellSize_)
                points.push_back(PointT(start.x, y));

            std::sort(points.begin(), points.end(), isBottomTop ? sort_y() : sort_reverse_y());
        }
        else
        {
            var isLeftRight = start.x < end.x;
            if (!isLeftRight)
            {
                T tmp = start;
                start = end;
                end = tmp;
            }

            int xStart = (int)std::ceil(start.x);
            int xEnd = (int)std::floor(end.X);
            for (int x = xStart; x <= xEnd; x += cellSize_)
                points.push_back(PointT(x, start.y));

            std::sort(points.begin(), points.end(), isLeftRight ? sort_x() : sort_reverse_x());
        }
    }

    void normalCase(PointT start, PointT end, double slope, Points& points)
    {
        double inverseSlope = 1 / slope;
        double b = start.Y - slope * start.X;

        bool isLeftRight = start.X < end.X;
        if (!isLeftRight)
        {
            T tmp = start;
            start = end;
            end = tmp;
        }

        int xStart = (int) std::ceil(start.X);
        int xEnd = (int) std::floor(end.X);
        for (int x = xStart; x <= xEnd; x += cellSize_)
            points.push_back(PointT(x, std::round(slope * x + b, roundDigitCount_)));

        bool isBottomTop = start.Y < end.Y;
        if (!isBottomTop)
        {
            T tmp = start;
            start = end;
            end = tmp;
        }

        int yStart = (int)std::ceil(start.Y);
        int yEnd = (int)std::floor(end.Y);
        for (int y = yStart; y <= yEnd; y += cellSize_)
            points.push_back(PointT(std::round((y - b) * inverseSlope, roundDigitCount_), y));

        std::sort(points.begin(), points.end(), isLeftRight ? sort_x() : sort_reverse_x());
    }

    Points filterResults(Points& points)
    {
        Points result;
        for (int i = 0; i < points.size(); ++i)
        {
            PointT candidate = points[i];
            if (result.size() > 0)
            {
                T last = result[result.size() - 1];
                double distance = std::sqrt((last.x - candidate.x) * (last.x - candidate.x) +
                    (last.y - candidate.y) * (last.y - candidate.y));
                if ((std::abs(distance) < std::numeric_limits<double>::epsilon))
                    continue;
            }

            result.push_back(candidate);
        }

        return std::move(result);
    }

    int cellSize_;
    int roundDigitCount_;
};

}}

#endif // TERRAIN_LINEGRIDSPLITTER_HPP_DEFINED
