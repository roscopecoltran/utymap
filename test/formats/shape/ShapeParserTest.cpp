#include "config.hpp"
#include "formats/shape/ShapeParser.hpp"
#include "formats/ShapeDataVisitor.hpp"

#include <boost/test/unit_test.hpp>

using namespace utymap::formats;

struct Formats_Shape_ShapeParserFixture
{
    Formats_Shape_ShapeParserFixture() { BOOST_TEST_MESSAGE("setup fixture"); }
    ~Formats_Shape_ShapeParserFixture() { BOOST_TEST_MESSAGE("teardown fixture"); }

    ShapeParser<ShapeDataVisitor> parser;
    ShapeDataVisitor visitor;
};

BOOST_FIXTURE_TEST_SUITE(Formats_ShapeParser, Formats_Shape_ShapeParserFixture)

BOOST_AUTO_TEST_CASE(GivenTestPointFile_WhenParse_ThenReturnNode)
{
    parser.parse(TEST_SHAPE_POINT_FILE, visitor);
}

BOOST_AUTO_TEST_SUITE_END()