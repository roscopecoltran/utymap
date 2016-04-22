#include "index/StringTable.hpp"

#include <boost/test/unit_test.hpp>

#include <cstdio>

using namespace utymap::index;

struct Index_StringTableFixture
{
    Index_StringTableFixture() :
        tablePtr(new StringTable(""))
    {
    }

    ~Index_StringTableFixture()
    {
        delete tablePtr;
        std::remove("string.idx");
        std::remove("string.dat");
    }
    StringTable* tablePtr;
};

BOOST_FIXTURE_TEST_SUITE( Index_StringTable, Index_StringTableFixture )

BOOST_AUTO_TEST_CASE( GivenNonPresentString_WhenGetIdFirstTime_ThenReturnZero )
{
    uint32_t id = tablePtr->getId("some_string");

    BOOST_CHECK_EQUAL( id, 0 );
}

BOOST_AUTO_TEST_CASE( GivenTable_WhenInsertMultiple_ThenReturnSequentialId )
{
    uint32_t id1 = tablePtr->getId("string1");
    uint32_t id2 = tablePtr->getId("string2");
    uint32_t id3 = tablePtr->getId("string3");

    BOOST_CHECK_EQUAL(id1, 0);
    BOOST_CHECK_EQUAL(id2, 1);
    BOOST_CHECK_EQUAL(id3, 2);
}

BOOST_AUTO_TEST_CASE(GivenThreeStrings_WhenGetIdOfSecond_ThenReturnValidId)
{
    tablePtr->getId("string1");
    tablePtr->getId("string2");
    tablePtr->getId("string3");

    std::uint32_t id = tablePtr->getId("string2");

    BOOST_CHECK_EQUAL(id, 1);
}

BOOST_AUTO_TEST_CASE( GivenThreeStrings_WhenGetStringOfSecond_ThenReturnValidString )
{
    tablePtr->getId("string1");
    tablePtr->getId("string2");
    tablePtr->getId("string3");

    std::string str = tablePtr->getString(1);

    BOOST_CHECK_EQUAL( str, "string2" );
}

BOOST_AUTO_TEST_SUITE_END()
