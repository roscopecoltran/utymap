#ifndef MAPCSS_COLOR_HPP_INCLUDED
#define MAPCSS_COLOR_HPP_INCLUDED

#include <algorithm>
#include <cstdint>

namespace utymap { namespace mapcss {

struct Color {

    Color(int r, int g, int b, int a) :
        r(std::max(0, std::min(r, 0xff))),
        g(std::max(0, std::min(g, 0xff))),
        b(std::max(0, std::min(b, 0xff))),
        a(std::max(0, std::min(a, 0xff)))
    {
    };

    Color(std::uint32_t rgba)
    {
        r = (rgba >> 24) & 0xff;
        g = (rgba >> 16) & 0xff;
        b = (rgba >>  8) & 0xff;
        a = (rgba >>  0) & 0xff;
    }

    operator std::uint32_t() const
    {
        return (r << 24) | (g << 16) | (b << 8) | a;
    }

    Color operator+(const Color& o) const 
    {
        return Color(r + o.r, g + o.g, b + o.b, a + o.a);
    }

    Color operator*(double f) const
    {
        return Color(r * f, g * f, b * f, a * f);
    }

    static Color mid(const Color& a, const Color& b, double r)
    {
        return (1.0 - r) * a + r * b;
    }
private:
    unsigned char r, g, b, a;
};

}}

#endif  // MAPCSS_COLORGRADIENT_HPP_INCLUDED
