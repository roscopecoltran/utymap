#ifndef UTILS_GRADIENTUTILS_HPP_DEFINED
#define UTILS_GRADIENTUTILS_HPP_DEFINED

#include "mapcss/ColorGradient.hpp"

#include <string>
#include <regex>

namespace utymap { namespace utils {

class GradientUtils
{
public:
    static utymap::mapcss::ColorGradient parse(const std::string& gradientStr);
private:
    static const std::regex gradientRegEx;
};

}}

#endif // UTILS_GRADIENTUTILS_HPP_DEFINED
