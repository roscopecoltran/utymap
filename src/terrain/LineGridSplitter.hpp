#ifndef TERRAIN_LINEGRIDSPLITTER_HPP_DEFINED
#define TERRAIN_LINEGRIDSPLITTER_HPP_DEFINED

#include "meshing/MeshTypes.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
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

    LineGridSplitter(uint8_t roundCount) : 
        roundVal_(std::pow(10, roundCount)),
        step_(1. / roundVal_)
    {
    }

    // Splits line to segments.
    Points split(PointT s, PointT e)
    {
        PointT start(s.x, s.y);
        PointT end(e.x, e.y);

        Points points(2);
        points.push_back(start);

        double slope = (end.y - start.y) / (end.x - start.x);
        if (std::isinf(slope) || std::abs(slope) < std::numeric_limits<double>::epsilon())
            zeroSlope(start, end, points);
        else
            normalCase(start, end, slope, points);

        return filterResults(points);
    }

private:
    void zeroSlope(PointT start, PointT end, Points& points)
    {
        if (std::abs(start.x - end.x) < std::numeric_limits<double>::epsilon())
        {
            bool isBottomTop = start.y < end.y;
            if (!isBottomTop)
            {
                PointT tmp = start;
                start = end;
                end = tmp;
            }

            // TODO
            double yStart = std::ceil(start.y * roundVal_) / roundVal_;
            double yEnd = std::floor(end.y * roundVal_) / roundVal_;
            for (double y = yStart; y <= yEnd; y += step_)
                points.push_back(PointT(start.x, y));

            if (isBottomTop)
                std::sort(points.begin(), points.end(), sort_y());
            else
                std::sort(points.begin(), points.end(), sort_reverse_y());
        }
        else
        {
            bool isLeftRight = start.x < end.x;
            if (!isLeftRight)
            {
                PointT tmp = start;
                start = end;
                end = tmp;
            }

            double xStart = std::ceil(start.x * roundVal_) / roundVal_;
            double xEnd = std::floor(end.x * roundVal_) / roundVal_;
            for (double x = xStart; x <= xEnd; x += step_)
                points.push_back(PointT(x, start.y));

            if (isLeftRight)
                std::sort(points.begin(), points.end(), sort_x());
            else
                std::sort(points.begin(), points.end(), sort_reverse_x());
        }
    }

    void normalCase(PointT start, PointT end, double slope, Points& points)
    {
        double inverseSlope = 1 / slope;
        double b = start.y - slope * start.x;

        bool isLeftRight = start.x < end.x;
        if (!isLeftRight)
        {
            PointT tmp = start;
            start = end;
            end = tmp;
        }

        double xStart = std::ceil(start.x * roundVal_) / roundVal_;
        double xEnd = std::floor(end.x * roundVal_) / roundVal_;
        for (double x = xStart; x <= xEnd; x += step_)
            points.push_back(PointT(x, slope * x + b));

        bool isBottomTop = start.y < end.y;
        if (!isBottomTop)
        {
            PointT tmp = start;
            start = end;
            end = tmp;
        }

        double yStart = std::ceil(start.y * roundVal_) / roundVal_;
        double yEnd = std::floor(end.y * roundVal_) / roundVal_;
        for (double y = yStart; y <= yEnd; y += step_)
            points.push_back(PointT((y - b) * inverseSlope, y));

        if (isLeftRight)
            std::sort(points.begin(), points.end(), sort_x());
        else
            std::sort(points.begin(), points.end(), sort_reverse_x());
    }

    Points filterResults(Points& points)
    {
        Points result;
        for (int i = 0; i < points.size(); ++i)
        {
            PointT candidate = points[i];
            if (result.size() > 0)
            {
                PointT last = result[result.size() - 1];
                double distance = std::sqrt((last.x - candidate.x) * (last.x - candidate.x) +
                    (last.y - candidate.y) * (last.y - candidate.y));
                if ((std::abs(distance) < std::numeric_limits<double>::epsilon()))
                    continue;
            }

            result.push_back(candidate);
        }

        return std::move(result);
    }

    uint32_t roundVal_;
    double step_;
};

}}

#endif // TERRAIN_LINEGRIDSPLITTER_HPP_DEFINED
