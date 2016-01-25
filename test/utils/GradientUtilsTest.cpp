#include "mapcss/ColorGradient.hpp"
#include "utils/GradientUtils.hpp"

#include <boost/test/unit_test.hpp>

#include <string>

using namespace utymap::mapcss;
using namespace utymap::utils;

BOOST_AUTO_TEST_SUITE(Utils_GradientUtils)

BOOST_AUTO_TEST_CASE(GivenSimpleGradientString_WhenParse_ThenReturnGradient)
{
     std::string gradientKey = "gradient(#0fffff, #099999 50%, #033333 70%, #000000)";

     ColorGradient gradient = GradientUtils::parse(gradientKey);

     BOOST_CHECK_GT(gradient.evaluate(0.5), 0);
}

BOOST_AUTO_TEST_SUITE_END()
