#ifndef MAPCSS_COLORGRADIENT_HPP_INCLUDED
#define MAPCSS_COLORGRADIENT_HPP_INCLUDED

#include <vector>
#include <utility>

namespace utymap { namespace mapcss {

// Represents color gradient.
class ColorGradient
{
    typedef std::vector<std::pair<int, int>> ColorData;
public:

    ColorGradient() {}

    ColorGradient(ColorData& colors) :
        colors_(std::move(colors))
    {
    }

    inline float evaluate(float time) const
    {
        // TODO
        return 0;
    }

private:
    ColorData colors_;
};

}}

#endif  // MAPCSS_COLORGRADIENT_HPP_INCLUDED
