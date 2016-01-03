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
        shapeFile(TEST_SHAPE_NE_110M_COASTLINE),
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
    ShapeDataVisitor visitor(*storePtr);
    ShapeParser<ShapeDataVisitor> parser;

    parser.parse(shapeFile, visitor);

    BOOST_CHECK_EQUAL(0, visitor.bounds);
    BOOST_CHECK_EQUAL(0, visitor.nodes);
    BOOST_CHECK_EQUAL(134, visitor.ways);
    BOOST_CHECK_EQUAL(0, visitor.relations);
}

BOOST_AUTO_TEST_SUITE_END()
