#include "mapcss/Parser.hpp"
#include <boost/test/unit_test.hpp>

using namespace utymap::mapcss;

BOOST_AUTO_TEST_SUITE( MapCss_Parser )

BOOST_AUTO_TEST_CASE( my_test )
{
    Parser parser(42);
    BOOST_CHECK(parser.getValue() == 42);
}

BOOST_AUTO_TEST_SUITE_END()
