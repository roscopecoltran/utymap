#include "config.hpp"
#include "formats/shape/ShapeParser.hpp"
#include "index/ShapeDataVisitor.hpp"
#include "index/InMemoryElementStore.hpp"
#include "mapcss/MapCssParser.hpp"
#include "mapcss/StyleSheet.hpp"
#include "mapcss/StyleProvider.hpp"

#include <boost/test/unit_test.hpp>
#include "test_utils/MapCssUtils.hpp"

using namespace utymap::index;
using namespace utymap::formats;
using namespace utymap::mapcss;

struct Formats_ShapeDataVisitorFixture
{
    Formats_ShapeDataVisitorFixture() :
        shapeFile(TEST_SHAPE_LINE_FILE),
        stringTablePtr(new StringTable("")),
        styleProviderPtr(MapCssUtils::createStyleProviderFromString(*stringTablePtr, "area|z1-16[test=Foo] { key:val; }"))
    {
        BOOST_TEST_MESSAGE("setup fixture");

        utymap::mapcss::Parser parser;
        std::ifstream styleFile(TEST_MAPCSS_DEFAULT);
        utymap::mapcss::StyleSheet stylesheet = parser.parse(styleFile);
        BOOST_TEST_CHECK(parser.getError().empty());
        storePtr = new InMemoryElementStore(*styleProviderPtr, *stringTablePtr);
    }

    ~Formats_ShapeDataVisitorFixture()
    {
        BOOST_TEST_MESSAGE("teardown fixture");
        delete stringTablePtr;
        delete styleProviderPtr;
        delete storePtr;
        std::remove("string.idx");
        std::remove("string.dat");
    }

    std::string shapeFile;
    StringTable* stringTablePtr;
    StyleProvider* styleProviderPtr;
    InMemoryElementStore* storePtr;
};

BOOST_FIXTURE_TEST_SUITE(Formats_ShapeDataVisitor, Formats_ShapeDataVisitorFixture)

BOOST_AUTO_TEST_CASE(GivenDefaultXml_WhenParserParse_ThenHasExpectedElementCount)
{
    ShapeDataVisitor visitor(*storePtr, *stringTablePtr);
    ShapeParser<ShapeDataVisitor> parser;

    parser.parse(shapeFile, visitor);

    BOOST_CHECK_EQUAL(0, visitor.nodes);
    BOOST_CHECK_EQUAL(0, visitor.ways);
    BOOST_CHECK_EQUAL(1, visitor.areas);
    BOOST_CHECK_EQUAL(0, visitor.relations);
}

BOOST_AUTO_TEST_SUITE_END()
