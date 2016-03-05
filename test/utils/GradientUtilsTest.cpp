#include "mapcss/ColorGradient.hpp"
#include "utils/GradientUtils.hpp"

#include <boost/test/unit_test.hpp>

#include <string>

using namespace utymap::mapcss;
using namespace utymap::utils;

BOOST_AUTO_TEST_SUITE(Utils_GradientUtils)

BOOST_AUTO_TEST_CASE(GivenSimpleGradientString_WhenEvaluate_ThenReturnCorrectColor)
{
     std::string gradientKey = "gradient(#0fffff, #099999 50%, #033333 70%, #000000)";
     ColorGradient gradient = GradientUtils::parseGradient(gradientKey);

     auto color = gradient.evaluate(0.5);

     BOOST_CHECK_EQUAL(color, 0x099999FF);
}

BOOST_AUTO_TEST_CASE(GivenHexColorString_WhenParse_ThenReturnColor)
{
    std::string colorStr = "#aa0000";

    Color color = GradientUtils::parseColor(colorStr);

    BOOST_CHECK_EQUAL(color, 0xAA0000FF);
}

BOOST_AUTO_TEST_CASE(GivenNamedColorString_WhenParse_ThenReturnColor)
{
    std::string colorStr = "red";

    std::uint32_t color = GradientUtils::parseColor(colorStr);

    BOOST_CHECK_EQUAL(color, 0xFF0000FF);
}

BOOST_AUTO_TEST_SUITE_END()
