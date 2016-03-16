#ifndef MAPCSS_COLORGRADIENT_HPP_INCLUDED
#define MAPCSS_COLORGRADIENT_HPP_INCLUDED

#include "mapcss/Color.hpp"

#include <cstdint>
#include <vector>
#include <utility>

namespace utymap { namespace mapcss {

// Represents color gradient.
class ColorGradient
{
public:

    // gradient data: first - time, second - color.
    typedef std::vector<std::pair<double, utymap::mapcss::Color>> GradientData;

    ColorGradient() {}

    ColorGradient(const GradientData& colors) :
        colors_(colors)
    {
    }

    inline utymap::mapcss::Color evaluate(double time) const
    {
        GradientData::size_type index = 0;
        while (index < colors_.size() - 1 && colors_[index].first < time)
            index++;

        auto pairA = colors_[index != 0 ? index - 1 : 0];
        auto pairB = colors_[index];

        double timeA = pairA.first;
        double timeB = pairB.first;
        double mu = index == 0 ? 0 : (time - timeA) / (timeB - timeA);

        return interpolate(pairA.second, pairB.second, mu);
    }

private:

    // So far, use linear interpolation algorithm as the fastest.
    inline utymap::mapcss::Color interpolate(const utymap::mapcss::Color& a,
                                             const utymap::mapcss::Color& b,
                                             double r) const
    {
        return a * (1.0 - r) + b * r;
    }

    GradientData colors_;
};

}}

#endif  // MAPCSS_COLORGRADIENT_HPP_INCLUDED
