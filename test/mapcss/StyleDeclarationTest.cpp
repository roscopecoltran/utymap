#include "entities/Node.hpp"
#include "index/StringTable.hpp"
#include "mapcss/StyleDeclaration.hpp"

#include <boost/test/unit_test.hpp>
#include "test_utils/ElementUtils.hpp"

#include <memory>

using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::mapcss;

namespace {
    struct Mapcss_StyleDeclarationFixture
    {
        Mapcss_StyleDeclarationFixture() :
            stringTable(new StringTable(""))
        {
        }

        ~Mapcss_StyleDeclarationFixture()
        {
            stringTable.reset();
            std::remove("string.idx");
            std::remove("string.dat");
        }

        std::shared_ptr<StringTable> stringTable;
    };
}

BOOST_FIXTURE_TEST_SUITE(Mapcss_StyleDeclaration, Mapcss_StyleDeclarationFixture)

BOOST_AUTO_TEST_CASE(GivenOnlySingleTag_WhenEvaluate_ReturnValue)
{
    StyleDeclaration styleDeclaration(0, "eval(tag(\"height\"))");

    double result = styleDeclaration.evaluate(
        ElementUtils::createElement<Node>(*stringTable, { { "height", "2.5" } }).tags, 
        *stringTable);

    BOOST_CHECK_EQUAL(result, 2.5);
}

BOOST_AUTO_TEST_SUITE_END()