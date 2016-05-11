#include "config.hpp"
#include "formats/shape/ShapeParser.hpp"
#include "formats/shape/ShapeDataVisitor.hpp"
#include "index/InMemoryElementStore.hpp"

#include <boost/test/unit_test.hpp>

#include "test_utils/MapCssUtils.hpp"

using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::formats;
using namespace utymap::mapcss;

struct Formats_Shape_ShapeDataVisitorFixture
{
    Formats_Shape_ShapeDataVisitorFixture() :
        shapeFile(TEST_SHAPE_LINE_FILE),
        stringTablePtr(new StringTable("")),
        styleProviderPtr(MapCssUtils::createStyleProviderFromString(*stringTablePtr, "way|z1-16[test=Foo] { key:val; }"))
    {
        storePtr = new InMemoryElementStore(*stringTablePtr);
    }

    ~Formats_Shape_ShapeDataVisitorFixture()
    {
        delete stringTablePtr;
        delete storePtr;

        std::remove("string.idx");
        std::remove("string.dat");
    }

    std::string shapeFile;
    StringTable* stringTablePtr;
    std::shared_ptr<StyleProvider> styleProviderPtr;
    InMemoryElementStore* storePtr;
};

BOOST_FIXTURE_TEST_SUITE(Formats_Shape_ShapeDataVisitor, Formats_Shape_ShapeDataVisitorFixture)

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
