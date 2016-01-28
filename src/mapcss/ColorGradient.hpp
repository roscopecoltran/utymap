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
    typedef std::vector<std::pair<float, utymap::mapcss::Color>> GradientData;

    ColorGradient() {}

    ColorGradient(GradientData& colors) :
        colors_(std::move(colors))
    {
    }

    inline utymap::mapcss::Color evaluate(float time) const
    {
        GradientData::size_type index = 0;
        for (; index < colors_.size(); ++index) {
            if (colors_[index].first > time) {
                break;
            }
        }

        auto pairA = colors_[index != 0 ? index - 1 : 0];
        auto pairB = colors_[index]; 

        float timeA = pairA.first;
        float timeB = pairB.first;
        float mu = index == 0 ? 0 : (time - timeA) / (timeB - timeA);

        return interpolate(pairA.second, pairB.second, mu);
    }

private:
    
    // So far, use linear interpolation algorithm as the fastest.
    inline utymap::mapcss::Color interpolate(const utymap::mapcss::Color& a, const utymap::mapcss::Color& b, double r) const
    {
        return (1.0 - r) * a + r * b;
    }

    GradientData colors_;
};

}}

#endif  // MAPCSS_COLORGRADIENT_HPP_INCLUDED
