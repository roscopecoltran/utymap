#include "config.hpp"
#include "formats/shape/ShapeParser.hpp"
#include "formats/shape/ShapeDataVisitor.hpp"
#include "index/InMemoryElementStore.hpp"
#include "LodRange.hpp"
#include "mapcss/StyleProvider.hpp"

#include <boost/test/unit_test.hpp>

#include "test_utils/MapCssUtils.hpp"

using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::formats;
using namespace utymap::mapcss;

struct Formats_ShapeDataVisitorFixture
{
    Formats_ShapeDataVisitorFixture() :
        shapeFile(TEST_SHAPE_LINE_FILE),
        stringTablePtr(new StringTable("")),
        styleProviderPtr(MapCssUtils::createStyleProviderFromString(*stringTablePtr, "way|z1-16[test=Foo] { key:val; }"))
    {
        BOOST_TEST_MESSAGE("setup fixture");
        storePtr = new InMemoryElementStore(*stringTablePtr);
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
    ShapeDataVisitor visitor(*stringTablePtr, [&](Element& element) {
        return storePtr->store(element, utymap::LodRange(1, 1), *styleProviderPtr);
    });
    ShapeParser<ShapeDataVisitor> parser;

    parser.parse(shapeFile, visitor);

    BOOST_CHECK_EQUAL(0, visitor.nodes);
    BOOST_CHECK_EQUAL(1, visitor.ways);
    BOOST_CHECK_EQUAL(0, visitor.areas);
    BOOST_CHECK_EQUAL(0, visitor.relations);
}

BOOST_AUTO_TEST_SUITE_END()
