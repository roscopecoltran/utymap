#include <boost/test/unit_test.hpp>
#include "mapcss/Parser.hpp"
#include "mapcss/Stylesheet.hpp"

#include <string>

using namespace utymap::mapcss;

BOOST_AUTO_TEST_SUITE( MapCss_Parser )

BOOST_AUTO_TEST_CASE( GivenMultilineComment_WhenParseIsCalled_ThenHasStylesheetAndNoError )
{
    std::string data = "/* Some comment */";
    Parser<std::string> parser;

    Stylesheet* stylesheet = parser.Parse(data);

    BOOST_CHECK(stylesheet != nullptr);
    BOOST_CHECK(parser.GetLastError().empty());
}

BOOST_AUTO_TEST_SUITE_END()
