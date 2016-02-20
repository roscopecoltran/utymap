#include "BoundingBox.hpp"
#include "formats/xml/OsmXmlParser.hpp"
#include "formats/TestOsmDataVisitor.hpp"
#include "config.hpp"

#include <boost/test/unit_test.hpp>

#include <fstream>

using namespace utymap::formats;

struct Formats_Xml_OsmXmlParserFixture
{
    Formats_Xml_OsmXmlParserFixture() :
        istream(TEST_XML_FILE, std::ios::in)
    { BOOST_TEST_MESSAGE("setup fixture"); }

    ~Formats_Xml_OsmXmlParserFixture() { BOOST_TEST_MESSAGE("teardown fixture"); }

    OsmXmlParser<TestOsmDataVisitor> parser;
    TestOsmDataVisitor visitor;
    std::ifstream istream;
};

BOOST_FIXTURE_TEST_SUITE( Formats_Xml_Parser, Formats_Xml_OsmXmlParserFixture )

BOOST_AUTO_TEST_CASE( GivenDefaultXml_WhenParserParse_ThenHasExpectedElementCount )
{
    parser.parse(istream, visitor);

    BOOST_CHECK_EQUAL(1, visitor.bounds);
    BOOST_CHECK_EQUAL(7653, visitor.nodes);
    BOOST_CHECK_EQUAL(1116, visitor.ways);
    BOOST_CHECK_EQUAL(92, visitor.relations);
}

BOOST_AUTO_TEST_SUITE_END()
