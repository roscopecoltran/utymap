#include "index/StringTable.hpp"

#include <boost/test/unit_test.hpp>
#include "test_utils/DependencyProvider.hpp"

#include <cstdio>

using namespace utymap::index;

namespace {
    struct Index_StringTableFixture
    {
        DependencyProvider depedencyProvider;
    };
}

BOOST_FIXTURE_TEST_SUITE(Index_StringTable, Index_StringTableFixture)

BOOST_AUTO_TEST_CASE(GivenNonPresentString_WhenGetIdFirstTime_ThenReturnZero)
{
    uint32_t id = depedencyProvider.getStringTable()->getId("some_string");

    BOOST_CHECK_EQUAL( id, 0 );
}

BOOST_AUTO_TEST_CASE( GivenTable_WhenInsertMultiple_ThenReturnSequentialId )
{
    uint32_t id1 = depedencyProvider.getStringTable()->getId("string1");
    uint32_t id2 = depedencyProvider.getStringTable()->getId("string2");
    uint32_t id3 = depedencyProvider.getStringTable()->getId("string3");

    BOOST_CHECK_EQUAL(id1, 0);
    BOOST_CHECK_EQUAL(id2, 1);
    BOOST_CHECK_EQUAL(id3, 2);
}

BOOST_AUTO_TEST_CASE(GivenThreeStrings_WhenGetIdOfSecond_ThenReturnValidId)
{
    depedencyProvider.getStringTable()->getId("string1");
    depedencyProvider.getStringTable()->getId("string2");
    depedencyProvider.getStringTable()->getId("string3");

    std::uint32_t id = depedencyProvider.getStringTable()->getId("string2");

    BOOST_CHECK_EQUAL(id, 1);
}

BOOST_AUTO_TEST_CASE(GivenThreeStrings_WhenGetStringOfSecond_ThenReturnValidString)
{
    depedencyProvider.getStringTable()->getId("string1");
    depedencyProvider.getStringTable()->getId("string2");
    depedencyProvider.getStringTable()->getId("string3");

    std::string str = depedencyProvider.getStringTable()->getString(1);

    BOOST_CHECK_EQUAL( str, "string2" );
}

BOOST_AUTO_TEST_SUITE_END()
