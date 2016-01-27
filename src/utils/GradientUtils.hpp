#ifndef UTILS_GRADIENTUTILS_HPP_DEFINED
#define UTILS_GRADIENTUTILS_HPP_DEFINED

#include "mapcss/Color.hpp"
#include "mapcss/ColorGradient.hpp"

#include <cstdint>
#include <string>
#include <regex>
#include <unordered_map>

namespace utymap { namespace utils {

// Provides the way to work with color gradient and color data.
class GradientUtils
{
public:

    // Parses color from string.
    static utymap::mapcss::Color parseColor(const std::string& colorStr);

    // Parses color gradient from string.
    static utymap::mapcss::ColorGradient parseGradient(const std::string& gradientStr);

private:
    static const std::regex gradientRegEx;
    static const std::unordered_map<std::string, utymap::mapcss::Color> colorMap;
};

}}

#endif // UTILS_GRADIENTUTILS_HPP_DEFINED
