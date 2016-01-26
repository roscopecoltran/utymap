#ifndef MAPCSS_COLORGRADIENT_HPP_INCLUDED
#define MAPCSS_COLORGRADIENT_HPP_INCLUDED

#include <cstdint>
#include <vector>
#include <utility>

namespace utymap { namespace mapcss {

// Represents color gradient.
class ColorGradient
{
public:

    // gradient data: first - time, second - color.
    typedef std::vector<std::pair<float, std::uint32_t>> GradientData;

    ColorGradient() {}

    ColorGradient(GradientData& colors) :
        colors_(std::move(colors))
    {
    }

    inline std::uint32_t evaluate(float time) const
    {
        GradientData::size_type index = 0;
        for (; index < colors_.size(); ++index) {
            if (colors_[index].first > time) {
                break;
            }
        }
        auto y1 = colors_[index != 0 ? index - 1 : 0].second;
        auto y2 = colors_[index].second;
        float mu = time;
        // TODO find correct mu
        return interpolate(y1, y2, time);
    }

private:
    GradientData colors_;

    // So far, using linear interpolation algorithm as the fastest.
    inline std::uint32_t interpolate(std::uint32_t y1, std::uint32_t y2, float mu) const
    { 
        return static_cast<std::uint32_t>(y1 * (1 - mu) + y2 * mu); 
    }
};

}}

#endif  // MAPCSS_COLORGRADIENT_HPP_INCLUDED
