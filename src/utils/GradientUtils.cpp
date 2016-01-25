#include "utils/GradientUtils.hpp"

#include <iostream>

using namespace utymap::mapcss;
using namespace utymap::utils;

// TODO assume that regex is thread safe
const std::regex GradientUtils::gradientRegEx = std::regex("rgb ?\\([ 0-9.%,]+?\\)|#[0-9a-fA-F]{3,6}\\s[0-9]{1,3}[%|px]|#[0-9a-fA-F]{3,6}|(aqua|black|blue|fuchsia|gray|green|lime|maroon|navy|olive|orange|purple|red|silver|teal|white|yellow){1}(\\s[0-9]{1,3}\\s*[%|px]?)?");

ColorGradient GradientUtils::parse(const std::string& gradientStr)
{
    /* TODO port from C#
    var results = MapCssGradientRegEx.Matches(gradientString);
            var count = results.Count;
            if (count == 0)
                throw new ArgumentException(String.Format(Strings.InvalidGradientString, gradientString));

            var colorKeys = new GradientWrapper.ColorKey[count];
            for (int i = 0; i < count; i++)
            {
                var values = results[i].Groups[0].Value.Split(' ');
                var color = ColorUtils.FromUnknown(values[0]);
                float time = i == 0 ? 0 :
                    (i == results.Count - 1) ? 1 :
                    float.Parse(values[1].Substring(0, values[1].Length - 1)) / 100f;
                colorKeys[i] = colorKeys[i] = new GradientWrapper.ColorKey
                {
                    Color = color.ToUnityColor(),
                    Time = time
                };
            }
    */
    auto begin = std::sregex_iterator(gradientStr.begin(), gradientStr.end(), gradientRegEx);
    auto end = std::sregex_iterator();

    std::vector<std::pair<int, int>> colors;
    colors.reserve(std::distance(begin, end));

    for (std::sregex_iterator i = begin; i != end; ++i) {
        std::smatch match = *i;
        std::string match_str = match.str();
        std::cout << match_str << '\n';
    }

    utymap::mapcss::ColorGradient gradient;
    return std::move(gradient);
}
