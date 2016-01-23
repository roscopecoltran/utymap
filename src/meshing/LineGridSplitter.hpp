#ifndef TERRAIN_LINEGRIDSPLITTER_HPP_DEFINED
#define TERRAIN_LINEGRIDSPLITTER_HPP_DEFINED

#include "clipper/clipper.hpp"
#include "meshing/MeshTypes.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

namespace utymap { namespace meshing {

// Splits line to segments according to axis aligned grid.
template<typename T>
class LineGridSplitter
{
    typedef utymap::meshing::Point<T> TPoint;
    typedef std::vector<TPoint> Points;

    typedef ClipperLib::IntPoint IPoint;
    typedef std::vector<IPoint> IPoints;

    struct sort_x
    {
        inline bool operator() (const IPoint& a, const IPoint& b) { return a.X < b.X; }
    };

    struct sort_reverse_x
    {
        inline bool operator() (const IPoint& a, const IPoint& b) { return a.X > b.X; }
    };

    struct sort_y
    {
        inline bool operator() (const IPoint& a, const IPoint& b) { return a.Y < b.Y; }
    };

    struct sort_reverse_y
    {
        inline bool operator() (const IPoint& a, const IPoint& b) { return a.Y > b.Y; }
    };

public:

    LineGridSplitter() :
        roundVal_(1),
        step_(1),
        scale_(1)
    {
    }

    void setRoundDigits(uint8_t roundDigits, uint8_t coeff = 1)
    {
        roundVal_ = std::pow(10, roundDigits);
        step_ = std::pow(10, roundDigits - 1) * coeff;
    }

    void setScale(uint64_t scale)
    {
        scale_ = scale;
    }

    // Splits line to segments.
    void split(IPoint start, IPoint end, Points& result)
    {
        // TODO reserve capacity
        std::vector<IPoint> points;
        points.push_back(start);

        double slope = (end.Y - start.Y) / ((double)end.X - start.X);
        if (std::isinf(slope) || std::abs(slope) < std::numeric_limits<double>::epsilon())
            zeroSlope(start, end, points);
        else
            normalCase(start, end, slope, points);

        filterResults(points, result);
    }

private:

    inline uint64_t ceil(uint64_t value)
    {
        return std::ceil(value / roundVal_) * roundVal_;
    }

    inline uint64_t floor(uint64_t value)
    {
        return std::floor(value / roundVal_) * roundVal_;
    }

    void zeroSlope(IPoint start, IPoint end, IPoints& points)
    {
        if ((start.X - end.X) == 0)
        {
            bool isBottomTop = start.Y < end.Y;
            if (!isBottomTop)
            {
                IPoint tmp = start;
                start = end;
                end = tmp;
            }

            uint64_t yStart = ceil(start.Y);
            uint64_t yEnd = floor(end.Y);
            for (double y = yStart; y <= yEnd; y += step_)
                points.push_back(IPoint(start.X, y));

            if (isBottomTop)
                std::sort(points.begin(), points.end(), sort_y());
            else
                std::sort(points.begin(), points.end(), sort_reverse_y());
        }
        else
        {
            bool isLeftRight = start.X < end.X;
            if (!isLeftRight)
            {
                IPoint tmp = start;
                start = end;
                end = tmp;
            }

            uint64_t xStart = ceil(start.X);
            uint64_t xEnd = floor(end.X);
            for (double x = xStart; x <= xEnd; x += step_)
                points.push_back(IPoint(x, start.Y));

            if (isLeftRight)
                std::sort(points.begin(), points.end(), sort_x());
            else
                std::sort(points.begin(), points.end(), sort_reverse_x());
        }
    }

    void normalCase(IPoint start, IPoint end, double slope, IPoints& points)
    {
        double inverseSlope = 1 / slope;
        double b = start.Y - slope * start.X;

        bool isLeftRight = start.X < end.X;
        if (!isLeftRight)
        {
            IPoint tmp = start;
            start = end;
            end = tmp;
        }

        uint64_t xStart = ceil(start.X);
        uint64_t xEnd = floor(end.X);
        for (double x = xStart; x <= xEnd; x += step_)
            points.push_back(IPoint(x, slope * x + b));

        bool isBottomTop = start.Y < end.Y;
        if (!isBottomTop)
        {
            IPoint tmp = start;
            start = end;
            end = tmp;
        }

        uint64_t yStart = ceil(start.Y);
        uint64_t yEnd = floor(end.Y);
        for (double y = yStart; y <= yEnd; y += step_)
            points.push_back(IPoint((y - b) * inverseSlope, y));

        if (isLeftRight)
            std::sort(points.begin(), points.end(), sort_x());
        else
            std::sort(points.begin(), points.end(), sort_reverse_x());
    }

    void filterResults(const IPoints& points, Points& result)
    {
        for (int i = 0; i < points.size(); ++i)
        {
            IPoint candidate = points[i];
            if (result.size() > 0)
            {
                TPoint last = result[result.size() - 1];
                uint64_t lastX = last.x * scale_;
                uint64_t lastY = last.y * scale_;
                double distance = std::sqrt((lastX - candidate.X) * (lastX - candidate.X) +
                                            (lastY - candidate.Y) * (lastY - candidate.Y));
                if (std::abs(distance) < 1)
                    continue;
            }

            result.push_back(TPoint(candidate.X / scale_, candidate.Y / scale_));
        }

        // NOTE do not allow first vertex to be equal the last one
        if (result[0] == result[result.size() - 1])
            result.erase(result.end() - 1);
    }

    uint32_t roundVal_;
    double scale_;
    uint64_t step_;
};

}}

#endif // TERRAIN_LINEGRIDSPLITTER_HPP_DEFINED
