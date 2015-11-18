#include "index/StringTable.hpp"

#include <boost/test/unit_test.hpp>

#include <cstdio>

using namespace utymap::index;

struct Index_StringTableFixture
{
    Index_StringTableFixture() :
        indexPath("index.idx"),
        stringPath("strings.dat"),
        table(*new StringTable(indexPath, stringPath))
    {
        BOOST_TEST_MESSAGE("setup fixture");
    }

    ~Index_StringTableFixture()
    {
        BOOST_TEST_MESSAGE("teardown fixture");
        delete &table;
        std::remove(indexPath.c_str());
        std::remove(stringPath.c_str());
    }

    std::string indexPath;
    std::string stringPath;
    StringTable& table;
};

BOOST_FIXTURE_TEST_SUITE( Index_StringTable, Index_StringTableFixture )

BOOST_AUTO_TEST_CASE( GivenNonPresentString_WhenGetIdFirstTime_ThenReturnZero )
{
    uint32_t id = table.getId("some_string");

    BOOST_CHECK( id == 0 );
}

BOOST_AUTO_TEST_CASE( GivenTable_WhenInsertMultiple_ThenReturnSequentialId )
{
    uint32_t id1 = table.getId("string1");
    uint32_t id2 = table.getId("string2");
    uint32_t id3 = table.getId("string3");

    BOOST_CHECK( id1 == 0 );
    BOOST_CHECK( id2 == 1 );
    BOOST_CHECK( id3 == 2 );
}

BOOST_AUTO_TEST_CASE( GivenThreeStrings_WhenGetIdOfSecond_ThenReturnValidId )
{
    uint32_t id = table.getId("string1");
    id = table.getId("string2");
    id = table.getId("string3");

    id = table.getId("string2");

    BOOST_CHECK( id == 1 );
}

BOOST_AUTO_TEST_CASE( GivenThreeStrings_WhenGetStringOfSecond_ThenReturnValidString )
{
    uint32_t id = table.getId("string1");
    table.getId("string2");
    table.getId("string3");

    std::string str = table.getString(1);

    BOOST_CHECK( str == "string2" );
}

BOOST_AUTO_TEST_SUITE_END()
