#include "config.hpp"
#include "index/ShapeDataVisitor.hpp"
#include "index/InMemoryElementStore.hpp"
#include "mapcss/MapCssParser.hpp"

#include <boost/test/unit_test.hpp>

using namespace utymap::index;
using namespace utymap::formats;

struct Formats_ShapeDataVisitorFixture
{
    Formats_ShapeDataVisitorFixture() :
        indexPath("index.idx"),
        stringPath("strings.dat"),
        shapeFile("g:/__ASM/_data/test/ne_110m_coastline"),
        stringTablePtr(new StringTable(indexPath, stringPath))
    {
        BOOST_TEST_MESSAGE("setup fixture");

        utymap::mapcss::Parser parser;
        styleFilterPtr = new StyleFilter(parser.parse(TEST_MAPCSS_DEFAULT), *stringTablePtr);
        storePtr = new InMemoryElementStore(*stringTablePtr, *styleFilterPtr);
    }

    ~Formats_ShapeDataVisitorFixture()
    {
        BOOST_TEST_MESSAGE("teardown fixture");
        delete styleFilterPtr;
        delete stringTablePtr;
        delete storePtr;
        std::remove(indexPath.c_str());
        std::remove(stringPath.c_str());
    }

    std::string indexPath;
    std::string stringPath;
    std::string shapeFile;
    StringTable* stringTablePtr;
    StyleFilter* styleFilterPtr;
    InMemoryElementStore* storePtr;
};

BOOST_FIXTURE_TEST_SUITE(Formats_ShapeDataVisitor, Formats_ShapeDataVisitorFixture)

BOOST_AUTO_TEST_CASE(GivenDefaultXml_WhenParserParse_ThenHasExpectedElementCount)
{
    ShapeDataVisitor shpVisitor(*storePtr);
    ShapeParser<ShapeDataVisitor> parser;

    parser.parse(shapeFile, shpVisitor);

    //BOOST_CHECK(visitor.bounds == 1);
    //BOOST_CHECK(visitor.nodes == 1263);
    //BOOST_CHECK(visitor.ways == 151);
    //BOOST_CHECK(visitor.relations == 54);
}

BOOST_AUTO_TEST_SUITE_END()
