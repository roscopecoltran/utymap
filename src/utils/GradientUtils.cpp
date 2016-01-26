#include "utils/GradientUtils.hpp"

#include <sstream>

using namespace utymap::mapcss;
using namespace utymap::utils;

// TODO assume that regex is thread safe
const std::regex GradientUtils::gradientRegEx = std::regex("rgb ?\\([ 0-9.%,]+?\\)|#[0-9a-fA-F]{3,6}\\s[0-9]{1,3}[%|px]|#[0-9a-fA-F]{3,6}|(aqua|black|blue|fuchsia|gray|green|lime|maroon|navy|olive|orange|purple|red|silver|teal|white|yellow){1}(\\s[0-9]{1,3}\\s*[%|px]?)?");

const std::unordered_map<std::string, std::uint32_t> colorMap = 
{
        { "activeborder", 0xB4B4B4FF },
        { "activecaption", 0x99B4D1FF },
        { "activecaptiontext", 0x0FF },
        { "appworkspace", 0xABABABFF },
        { "control", 0xF0F0F0FF },
        { "controldark", 0xA0A0A0FF },
        { "controldarkdark", 0x696969FF },
        { "controllight", 0xE3E3E3FF },
        { "controllightlight", 0xFFFFFFFF },
        { "controltext", 0x0FF },
        { "desktop", 0x0FF },
        { "graytext", 0x6D6D6DFF },
        { "highlight", 0x3399FFFF },
        { "highlighttext", 0xFFFFFFFF },
        { "hottrack", 0x66CCFF },
        { "inactiveborder", 0xF4F7FCFF },
        { "inactivecaption", 0xBFCDDBFF },
        { "inactivecaptiontext", 0x0FF },
        { "info", 0xFFFFE1FF },
        { "infotext", 0x0FF },
        { "menu", 0xF0F0F0FF },
        { "menutext", 0x0FF },
        { "scrollbar", 0xC8C8C8FF },
        { "window", 0xFFFFFFFF },
        { "windowframe", 0x646464FF },
        { "windowtext", 0x0FF },
        { "transparent", 0xFFFFFFFF },
        { "aliceblue", 0xF0F8FFFF },
        { "antiquewhite", 0xFAEBD7FF },
        { "aqua", 0xFFFFFF },
        { "aquamarine", 0x7FFFD4FF },
        { "azure", 0xF0FFFFFF },
        { "beige", 0xF5F5DCFF },
        { "bisque", 0xFFE4C4FF },
        { "black", 0x0FF },
        { "blanchedalmond", 0xFFEBCDFF },
        { "blue", 0xFFFF },
        { "blueviolet", 0x8A2BE2FF },
        { "brown", 0xA52A2AFF },
        { "burlywood", 0xDEB887FF },
        { "cadetblue", 0x5F9EA0FF },
        { "chartreuse", 0x7FFF00FF },
        { "chocolate", 0xD2691EFF },
        { "coral", 0xFF7F50FF },
        { "cornflowerblue", 0x6495EDFF },
        { "cornsilk", 0xFFF8DCFF },
        { "crimson", 0xDC143CFF },
        { "cyan", 0xFFFFFF },
        { "darkblue", 0x8BFF },
        { "darkcyan", 0x8B8BFF },
        { "darkgoldenrod", 0xB8860BFF },
        { "darkgray", 0xA9A9A9FF },
        { "darkgreen", 0x6400FF },
        { "darkkhaki", 0xBDB76BFF },
        { "darkmagenta", 0x8B008BFF },
        { "darkolivegreen", 0x556B2FFF },
        { "darkorange", 0xFF8C00FF },
        { "darkorchid", 0x9932CCFF },
        { "darkred", 0x8B0000FF },
        { "darksalmon", 0xE9967AFF },
        { "darkseagreen", 0x8FBC8BFF },
        { "darkslateblue", 0x483D8BFF },
        { "darkslategray", 0x2F4F4FFF },
        { "darkturquoise", 0xCED1FF },
        { "darkviolet", 0x9400D3FF },
        { "deeppink", 0xFF1493FF },
        { "deepskyblue", 0xBFFFFF },
        { "dimgray", 0x696969FF },
        { "dodgerblue", 0x1E90FFFF },
        { "firebrick", 0xB22222FF },
        { "floralwhite", 0xFFFAF0FF },
        { "forestgreen", 0x228B22FF },
        { "fuchsia", 0xFF00FFFF },
        { "gainsboro", 0xDCDCDCFF },
        { "ghostwhite", 0xF8F8FFFF },
        { "gold", 0xFFD700FF },
        { "goldenrod", 0xDAA520FF },
        { "gray", 0x808080FF },
        { "green", 0x8000FF },
        { "greenyellow", 0xADFF2FFF },
        { "honeydew", 0xF0FFF0FF },
        { "hotpink", 0xFF69B4FF },
        { "indianred", 0xCD5C5CFF },
        { "indigo", 0x4B0082FF },
        { "ivory", 0xFFFFF0FF },
        { "khaki", 0xF0E68CFF },
        { "lavender", 0xE6E6FAFF },
        { "lavenderblush", 0xFFF0F5FF },
        { "lawngreen", 0x7CFC00FF },
        { "lemonchiffon", 0xFFFACDFF },
        { "lightblue", 0xADD8E6FF },
        { "lightcoral", 0xF08080FF },
        { "lightcyan", 0xE0FFFFFF },
        { "lightgoldenrodyellow", 0xFAFAD2FF },
        { "lightgray", 0xD3D3D3FF },
        { "lightgreen", 0x90EE90FF },
        { "lightpink", 0xFFB6C1FF },
        { "lightsalmon", 0xFFA07AFF },
        { "lightseagreen", 0x20B2AAFF },
        { "lightskyblue", 0x87CEFAFF },
        { "lightslategray", 0x778899FF },
        { "lightsteelblue", 0xB0C4DEFF },
        { "lightyellow", 0xFFFFE0FF },
        { "lime", 0xFF00FF },
        { "limegreen", 0x32CD32FF },
        { "linen", 0xFAF0E6FF },
        { "magenta", 0xFF00FFFF },
        { "maroon", 0x800000FF },
        { "mediumaquamarine", 0x66CDAAFF },
        { "mediumblue", 0xCDFF },
        { "mediumorchid", 0xBA55D3FF },
        { "mediumpurple", 0x9370DBFF },
        { "mediumseagreen", 0x3CB371FF },
        { "mediumslateblue", 0x7B68EEFF },
        { "mediumspringgreen", 0xFA9AFF },
        { "mediumturquoise", 0x48D1CCFF },
        { "mediumvioletred", 0xC71585FF },
        { "midnightblue", 0x191970FF },
        { "mintcream", 0xF5FFFAFF },
        { "mistyrose", 0xFFE4E1FF },
        { "moccasin", 0xFFE4B5FF },
        { "navajowhite", 0xFFDEADFF },
        { "navy", 0x80FF },
        { "oldlace", 0xFDF5E6FF },
        { "olive", 0x808000FF },
        { "olivedrab", 0x6B8E23FF },
        { "orange", 0xFFA500FF },
        { "orangered", 0xFF4500FF },
        { "orchid", 0xDA70D6FF },
        { "palegoldenrod", 0xEEE8AAFF },
        { "palegreen", 0x98FB98FF },
        { "paleturquoise", 0xAFEEEEFF },
        { "palevioletred", 0xDB7093FF },
        { "papayawhip", 0xFFEFD5FF },
        { "peachpuff", 0xFFDAB9FF },
        { "peru", 0xCD853FFF },
        { "pink", 0xFFC0CBFF },
        { "plum", 0xDDA0DDFF },
        { "powderblue", 0xB0E0E6FF },
        { "purple", 0x800080FF },
        { "red", 0xFF0000FF },
        { "rosybrown", 0xBC8F8FFF },
        { "royalblue", 0x4169E1FF },
        { "saddlebrown", 0x8B4513FF },
        { "salmon", 0xFA8072FF },
        { "sandybrown", 0xF4A460FF },
        { "seagreen", 0x2E8B57FF },
        { "seashell", 0xFFF5EEFF },
        { "sienna", 0xA0522DFF },
        { "silver", 0xC0C0C0FF },
        { "skyblue", 0x87CEEBFF },
        { "slateblue", 0x6A5ACDFF },
        { "slategray", 0x708090FF },
        { "snow", 0xFFFAFAFF },
        { "springgreen", 0xFF7FFF },
        { "steelblue", 0x4682B4FF },
        { "tan", 0xD2B48CFF },
        { "teal", 0x8080FF },
        { "thistle", 0xD8BFD8FF },
        { "tomato", 0xFF6347FF },
        { "turquoise", 0x40E0D0FF },
        { "violet", 0xEE82EEFF },
        { "wheat", 0xF5DEB3FF },
        { "white", 0xFFFFFFFF },
        { "whitesmoke", 0xF5F5F5FF },
        { "yellow", 0xFFFF00FF },
        { "yellowgreen", 0x9ACD32FF },
        { "buttonface", 0xF0F0F0FF },
        { "buttonhighlight", 0xFFFFFFFF },
        { "buttonshadow", 0xA0A0A0FF },
        { "gradientactivecaption", 0xB9D1EAFF },
        { "gradientinactivecaption", 0xD7E4F2FF },
        { "menubar", 0xF0F0F0FF },
        { "menuhighlight", 0x3399FFFF },
        { "grey", 0xBEBEBEFF },
};

char hexToInt(char hexChar)
{
    switch (hexChar)
    {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        case 'a': return 10;
        case 'b': return 11;
        case 'c': return 12;
        case 'd': return 13;
        case 'e': return 14;
        case 'f': return 15;
    }
    throw std::invalid_argument(std::string("Unknown hex:") + hexChar);
}

std::uint32_t fromName(const std::string& colorStr)
{
    // TODO check existence
    return colorMap.find(colorStr)->second;
}

// Gets integer representation of color from color string.
std::uint32_t fromHex(const std::string& colorStr)
{
    std::uint32_t value = hexToInt(colorStr[2]) + hexToInt(colorStr[1]) * 16; // red
    value = (value << 8) + hexToInt(colorStr[4]) + hexToInt(colorStr[3]) * 16; // green
    value = (value << 8) + hexToInt(colorStr[6]) + hexToInt(colorStr[5]) * 16; // blue
    return (value << 8) + 0xFF; // alpha: NOTE always opaque
}

ColorGradient GradientUtils::parseGradient(const std::string& gradientStr)
{
    auto begin = std::sregex_iterator(gradientStr.begin(), gradientStr.end(), gradientRegEx);
    auto end = std::sregex_iterator();

    ColorGradient::GradientData data;
    data.reserve(std::distance(begin, end));

    for (std::sregex_iterator iter = begin; iter != end; ++iter) {
        std::stringstream ss(iter->str());
        std::pair<float, std::uint32_t> pair(iter == begin ? 0 : 1, 0);
        std::string partStr;
        int part = 0;
        while (std::getline(ss, partStr, ' ')) {
            if (part++ == 0) {
                pair.second = parseColor(partStr);
                continue;
            }
            // NOTE assume that for all intermediate colors time is always defined
            // and skipped for first and last.
            pair.first = std::stof(partStr.substr(0, partStr.size() - 1)) / 100.;
        }
        data.push_back(pair);
    }

    return std::move(utymap::mapcss::ColorGradient(data));
}

std::uint32_t GradientUtils::parseColor(const std::string& colorStr)
{
    return colorStr[0] == '#' ?  fromHex(colorStr) : fromName(colorStr);
}
