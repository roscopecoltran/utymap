#include "formats/Types.hpp"
#include "formats/xml/OsmXmlParser.hpp"
#include "formats/OsmDataVisitor.hpp"
#include "config.hpp"

#include <boost/test/unit_test.hpp>

#include <fstream>

using namespace utymap::formats;

struct XmlParserFixture 
{
    XmlParserFixture() : istream(TEST_XML_FILE, std::ios::in)
    {
        BOOST_TEST_MESSAGE("setup fixture");
    }

    ~XmlParserFixture() { BOOST_TEST_MESSAGE("teardown fixture"); }

    OsmXmlParser<OsmDataVisitor> parser;
    OsmDataVisitor visitor;
    std::ifstream istream;
};

BOOST_FIXTURE_TEST_SUITE(Formats_Xml_Parser, XmlParserFixture)

BOOST_AUTO_TEST_CASE( GivenDefaultXml_WhenParserParse_ThenHasExpectedElementCount )
{
    parser.parse(istream, visitor);

    BOOST_CHECK( visitor.bounds == 1 );
    BOOST_CHECK( visitor.nodes == 1263 );
    BOOST_CHECK( visitor.ways == 151 );
    BOOST_CHECK( visitor.relations == 54 );
}

BOOST_AUTO_TEST_SUITE_END()
