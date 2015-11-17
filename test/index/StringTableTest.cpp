#include "index/StringTable.hpp"

#include <boost/test/unit_test.hpp>

using namespace utymap::index;

struct Index_StringTableFixture
{
    Index_StringTableFixture() : 
        indexPath("index.idx"), 
        stringPath("strings.dat"), 
        table(indexPath, stringPath)
    { BOOST_TEST_MESSAGE("setup fixture"); }
    ~Index_StringTableFixture() 
    { 
        BOOST_TEST_MESSAGE("teardown fixture"); 
        std::remove("index.idx");
        std::remove("strings.dat");
    }

    std::string indexPath;
    std::string stringPath;
    StringTable table;
};

BOOST_FIXTURE_TEST_SUITE( Index_StringTable, Index_StringTableFixture )

BOOST_AUTO_TEST_CASE( GivenNonPresentString_WhenGetIdFirstTime_ThenReturnZero )
{
    uint32_t id = table.getId("some_string");

    BOOST_CHECK( id == 0 );
}

BOOST_AUTO_TEST_SUITE_END()