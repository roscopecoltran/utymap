#ifndef UTILS_GRADIENTUTILS_HPP_DEFINED
#define UTILS_GRADIENTUTILS_HPP_DEFINED

#include "mapcss/ColorGradient.hpp"

#include <regex>

namespace utymap { namespace utils {

class GradientUtils
{
public:

    static utymap::mapcss::ColorGradient parse(const std::string& gradientStr)
    {
        utymap::mapcss::ColorGradient gradient;
        return std::move(gradient);
    }
};

}}

#endif // UTILS_GRADIENTUTILS_HPP_DEFINED
