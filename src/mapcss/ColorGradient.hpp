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

    inline float evaluate(float time) const
    {
        // TODO
        return 0;
    }

private:
    GradientData colors_;
};

}}

#endif  // MAPCSS_COLORGRADIENT_HPP_INCLUDED
