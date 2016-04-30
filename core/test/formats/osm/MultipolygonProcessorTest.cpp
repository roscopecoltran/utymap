#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "formats/osm/MultipolygonProcessor.hpp"
#include "test_utils/ElementUtils.hpp"

#include <boost/test/unit_test.hpp>

#include <cstdio>

using namespace utymap;
using namespace utymap::entities;
using namespace utymap::formats;
using namespace utymap::index;

struct Formats_Osm_MultipolygonProcessorFixture
{
    Formats_Osm_MultipolygonProcessorFixture() : 
        stringTablePtr(new StringTable("")), areaMap(), wayMap()
    {
    }

    ~Formats_Osm_MultipolygonProcessorFixture()
    {
        delete stringTablePtr;
        std::remove("string.idx");
        std::remove("string.dat");
    }

    RelationMembers createRelationMembers(const std::initializer_list<std::tuple<std::uint64_t, std::string, std::string>>& membersInfo)
    {
        RelationMembers members;
        members.reserve(membersInfo.size());
        for (const auto& info : membersInfo) {
            members.push_back(RelationMember{ std::get<0>(info), std::get<1>(info), std::get<2>(info) });
        }
        return std::move(members);
    }

    StringTable* stringTablePtr;
    std::unordered_map<std::uint64_t, std::shared_ptr<Area>> areaMap;
    std::unordered_map<std::uint64_t, std::shared_ptr<Way>> wayMap;
};

BOOST_FIXTURE_TEST_SUITE(Formats_Osm_MultipolygonProcessor, Formats_Osm_MultipolygonProcessorFixture)

BOOST_AUTO_TEST_CASE(GivenOneOuterOneInnerAllClosed_WhenProcess_ThenReturnCorrectResult)
{
    RelationMembers relationMembers = createRelationMembers({
        std::make_tuple(1, "way", "outer"),
        std::make_tuple(2, "way", "inner")
    });
    Tags tags = { utymap::formats::Tag{ "type", "multipolygon" }, utymap::formats::Tag{ "tag", "tags" } };
    areaMap[1] = std::shared_ptr<Area>(new Area(ElementUtils::createElement<Area>(*stringTablePtr, {},
                    { { 0, 0 }, { 3, 5 }, {7, 3}, {8, -1}, {3, -4}, {0, 0} })));
    areaMap[2] = std::shared_ptr<Area>(new Area(ElementUtils::createElement<Area>(*stringTablePtr, {},
                    { { 2, 0 }, { 3, 2 }, { 5, 1 }, { 4, -1 }, { 2, 0 } })));
    MultipolygonProcessor processor(0, relationMembers, tags, *stringTablePtr, areaMap, wayMap);

    Relation relation = processor.process();

    BOOST_CHECK_EQUAL(2, relation.elements.size());

    BOOST_CHECK_EQUAL(6, reinterpret_cast<const Area&>(*relation.elements[0]).coordinates.size());
    BOOST_CHECK(areaMap[1]->coordinates == reinterpret_cast<const Area&>(*relation.elements[0]).coordinates);

    BOOST_CHECK_EQUAL(5, reinterpret_cast<const Area&>(*relation.elements[1]).coordinates.size());
    std::reverse(areaMap[2]->coordinates.begin(), areaMap[2]->coordinates.end());
    BOOST_CHECK(areaMap[2]->coordinates == reinterpret_cast<const Area&>(*relation.elements[1]).coordinates);
}

BOOST_AUTO_TEST_CASE(GivenOneOuterTwoInnerAllClosed_WhenProcess_ThenReturnCorrectResult)
{
    RelationMembers relationMembers = createRelationMembers({
        std::make_tuple(1, "way", "outer"),
        std::make_tuple(2, "way", "inner"),
        std::make_tuple(3, "way", "inner"),
    });
    Tags tags = { utymap::formats::Tag{ "type", "multipolygon" }, utymap::formats::Tag{ "tag", "tags" } };
    areaMap[1] = std::shared_ptr<Area>(new Area(ElementUtils::createElement<Area>(*stringTablePtr, {},
                    { { 0, 0 }, { 3, 5 }, { 7, 3 }, { 8, -1 }, { 3, -4 }, { 0, 0 } })));
    areaMap[2] = std::shared_ptr<Area>(new Area(ElementUtils::createElement<Area>(*stringTablePtr, {},
                    { { 2, 1 }, { 3, 3 }, { 5, 2 }, { 4, 0 }, { 2, 1 } })));
    areaMap[3] = std::shared_ptr<Area>(new Area(ElementUtils::createElement<Area>(*stringTablePtr, {},
                    { { 3, -1 }, { 5, -1 }, { 3, -3 }, { 2, -2 }, { 3, -1 } })));
    MultipolygonProcessor processor(0, relationMembers, tags, *stringTablePtr, areaMap, wayMap);

    Relation relation = processor.process();

    BOOST_CHECK_EQUAL(3, relation.elements.size());

    BOOST_CHECK_EQUAL(6, reinterpret_cast<const Area&>(*relation.elements[0]).coordinates.size());
    BOOST_CHECK(areaMap[1]->coordinates == reinterpret_cast<const Area&>(*relation.elements[0]).coordinates);

    BOOST_CHECK_EQUAL(5, reinterpret_cast<const Area&>(*relation.elements[1]).coordinates.size());
    std::reverse(areaMap[2]->coordinates.begin(), areaMap[2]->coordinates.end());
    BOOST_CHECK(areaMap[2]->coordinates == reinterpret_cast<const Area&>(*relation.elements[1]).coordinates);

    BOOST_CHECK_EQUAL(5, reinterpret_cast<const Area&>(*relation.elements[2]).coordinates.size());
    std::reverse(areaMap[3]->coordinates.begin(), areaMap[3]->coordinates.end());
    BOOST_CHECK(areaMap[3]->coordinates == reinterpret_cast<const Area&>(*relation.elements[2]).coordinates);
}

BOOST_AUTO_TEST_CASE(GivenOneOuterNonClosed_WhenProcess_ThenReturnCorrectResult)
{
    RelationMembers relationMembers = createRelationMembers({
        std::make_tuple(1, "way", "outer"),
        std::make_tuple(2, "way", "outer")
    });
    Tags tags = { utymap::formats::Tag{ "type", "multipolygon" }, utymap::formats::Tag{ "tag", "tags" } };
    wayMap[1] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*stringTablePtr, {},
                    { { 0, 0 }, { 3, 5 }, { 7, 3 } })));
    wayMap[2] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*stringTablePtr, {},
                    { { 7, 3 }, { 8, -1 }, { 3, -4 }, { 0, 0 } })));
    MultipolygonProcessor processor(0, relationMembers, tags, *stringTablePtr, areaMap, wayMap);

    Relation relation = processor.process();

    BOOST_CHECK_EQUAL(1, relation.elements.size());
    BOOST_CHECK_EQUAL(6, reinterpret_cast<const Area&>(*relation.elements[0]).coordinates.size());
    std::vector<GeoCoordinate> expected = { { 7, 3 }, { 8, -1 }, { 3, -4 }, { 0, 0 }, { 3, 5 }, { 7, 3 } };
    BOOST_CHECK(expected == reinterpret_cast<const Area&>(*relation.elements[0]).coordinates);
}

BOOST_AUTO_TEST_CASE(GivenTwoOuterClosed_WhenProcess_ThenReturnCorrectResult)
{
    RelationMembers relationMembers = createRelationMembers({
        std::make_tuple(1, "way", "outer"),
        std::make_tuple(2, "way", "outer")
    });
    Tags tags = { utymap::formats::Tag{ "type", "multipolygon" }, utymap::formats::Tag{ "tag", "tags" } };
    areaMap[1] = std::shared_ptr<Area>(new Area(ElementUtils::createElement<Area>(*stringTablePtr, {},
                    { { 0, 0 }, { 3, 5 }, { 7, 3 }, { 8, -1 }, { 3, -4 }, { 0, 0 } })));
    areaMap[2] = std::shared_ptr<Area>(new Area(ElementUtils::createElement<Area>(*stringTablePtr, {},
                    { { 10, -3 }, { 14, -3 }, { 14, -6 }, { 10, -6 }, { 10, -3 } })));
    MultipolygonProcessor processor(0, relationMembers, tags, *stringTablePtr, areaMap, wayMap);

    Relation relation = processor.process();

    BOOST_CHECK_EQUAL(2, relation.elements.size());
    BOOST_CHECK(areaMap[1]->coordinates == reinterpret_cast<const Area&>(*relation.elements[1]).coordinates);
    BOOST_CHECK(areaMap[2]->coordinates == reinterpret_cast<const Area&>(*relation.elements[0]).coordinates);
}

BOOST_AUTO_TEST_CASE(GivenOneOuterNonClosedAndTwoInnerClosed_WhenProcess_ThenReturnCorrectResult)
{
    RelationMembers relationMembers = createRelationMembers({
        std::make_tuple(1, "way", "outer"),
        std::make_tuple(2, "way", "outer"),
        std::make_tuple(3, "way", "inner"),
        std::make_tuple(4, "way", "inner")
    });
    Tags tags = { utymap::formats::Tag{ "type", "multipolygon" }, utymap::formats::Tag{ "tag", "tags" } };
    wayMap[1] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*stringTablePtr, {},
                    { { 0, 0 }, { 3, 5 }, { 7, 3 } })));
    wayMap[2] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*stringTablePtr, {},
                    { { 7, 3 }, { 8, -1 }, { 3, -4 }, { 0, 0 } })));
    areaMap[3] = std::shared_ptr<Area>(new Area(ElementUtils::createElement<Area>(*stringTablePtr, {},
                    { { 2, 1 }, { 3, 3 }, { 5, 2 }, { 4, 0 }, { 2, 1 } })));
    areaMap[4] = std::shared_ptr<Area>(new Area(ElementUtils::createElement<Area>(*stringTablePtr, {},
                    { { 3, -1 }, { 5, -1 }, { 3, -3 }, { 2, -2 }, { 3, -1 } })));
    MultipolygonProcessor processor(0, relationMembers, tags, *stringTablePtr, areaMap, wayMap);

    Relation relation = processor.process();

    BOOST_CHECK_EQUAL(3, relation.elements.size());

    BOOST_CHECK_EQUAL(6, reinterpret_cast<const Area&>(*relation.elements[0]).coordinates.size());
    std::vector<GeoCoordinate> expected = { { 7, 3 }, { 8, -1 }, { 3, -4 }, { 0, 0 }, { 3, 5 }, { 7, 3 } };
    BOOST_CHECK(expected == reinterpret_cast<const Area&>(*relation.elements[0]).coordinates);

    BOOST_CHECK_EQUAL(5, reinterpret_cast<const Area&>(*relation.elements[2]).coordinates.size());
    std::reverse(areaMap[3]->coordinates.begin(), areaMap[3]->coordinates.end());
    BOOST_CHECK(areaMap[3]->coordinates == reinterpret_cast<const Area&>(*relation.elements[2]).coordinates);

    BOOST_CHECK_EQUAL(5, reinterpret_cast<const Area&>(*relation.elements[1]).coordinates.size());
    std::reverse(areaMap[4]->coordinates.begin(), areaMap[4]->coordinates.end());
    BOOST_CHECK(areaMap[4]->coordinates == reinterpret_cast<const Area&>(*relation.elements[1]).coordinates);
}

BOOST_AUTO_TEST_CASE(GivenMultiplyOuterAndMultiplyInner_WhenProcess_ThenReturnCorrectResult)
{
    // see fig.6 http://wiki.openstreetmap.org/wiki/Talk:Relation:multipolygon

    RelationMembers relationMembers = createRelationMembers({
        std::make_tuple(1, "way", "outer"),
        std::make_tuple(2, "way", "outer"),
        std::make_tuple(3, "way", "outer"),
        std::make_tuple(4, "way", "outer"),

        std::make_tuple(5, "way", "inner"),
        std::make_tuple(6, "way", "inner"),
        std::make_tuple(7, "way", "inner"),
        std::make_tuple(8, "way", "inner"),
        std::make_tuple(9, "way", "inner"),
        std::make_tuple(10, "way", "inner"),
        std::make_tuple(11, "way", "inner"),

        std::make_tuple(12, "way", "outer"),
        std::make_tuple(13, "way", "outer"),
        std::make_tuple(14, "way", "outer"),
        std::make_tuple(15, "way", "outer"),

        std::make_tuple(16, "way", "inner"),
        std::make_tuple(17, "way", "inner"),
        std::make_tuple(18, "way", "inner"),
        std::make_tuple(19, "way", "inner"),

        std::make_tuple(20, "way", "outer")
    });
    Tags tags = { utymap::formats::Tag{ "type", "multipolygon" }, utymap::formats::Tag{ "tag", "tags" } };
    wayMap[1] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*stringTablePtr, {}, { { 1, 5 }, { 8, 4 }})));
    wayMap[2] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*stringTablePtr, {}, { { 8, 4 }, { 9, -1 }})));
    wayMap[3] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*stringTablePtr, {}, { { 9, -1 }, { 8, -6 }, { 2, -5 } })));
    wayMap[4] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*stringTablePtr, {}, { { 2, -5 }, { 0, -3 }, { 1, 5 } })));

    wayMap[5] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*stringTablePtr, {}, { { 2, 1 }, { 3, 3 }, { 6, 3 } })));
    wayMap[6] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*stringTablePtr, {}, { { 6, 3 }, { 4, 0 }, { 2, 1 } })));
    wayMap[7] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*stringTablePtr, {}, { { 1, -2 }, { 3, -1 } })));
    wayMap[8] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*stringTablePtr, {}, { { 3, -1 }, { 4, -4 } })));
    wayMap[9] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*stringTablePtr, {}, { { 4, -4 }, { 1, -3 } })));
    wayMap[10] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*stringTablePtr, {}, { { 1, -3 }, { 1, -2 } })));
    areaMap[11] = std::shared_ptr<Area>(new Area(ElementUtils::createElement<Area>(*stringTablePtr, {}, { { 6, -3 }, { 7, -1 }, { 8, -4 }, { 6, -3 } })));

    wayMap[12] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*stringTablePtr, {}, { { 10, 5 }, { 14, 5 } })));
    wayMap[13] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*stringTablePtr, {}, { { 14, 5 }, { 14, -1 } })));
    wayMap[14] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*stringTablePtr, {}, { { 14, -1 }, { 10, -1 } })));
    wayMap[15] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*stringTablePtr, {}, { { 10, -1 }, { 10, 5 } })));

    wayMap[16] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*stringTablePtr, {}, { { 11, 4 }, { 13, 4 } })));
    wayMap[17] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*stringTablePtr, {}, { { 13, 4 }, { 13, 0 }, { 11, 0 } })));
    wayMap[18] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*stringTablePtr, {}, { { 11, 0 }, { 12, 2 } })));
    wayMap[19] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*stringTablePtr, {}, { { 12, 2 }, { 11, 4 } })));

    areaMap[20] = std::shared_ptr<Area>(new Area(ElementUtils::createElement<Area>(*stringTablePtr, {}, { { 10, -3 }, { 14, -3 }, { 14, -6 }, { 10, -6 }, { 10, -3 } })));
    MultipolygonProcessor processor(0, relationMembers, tags, *stringTablePtr, areaMap, wayMap);

    Relation relation = processor.process();

    BOOST_CHECK_EQUAL(7, relation.elements.size());
}

BOOST_AUTO_TEST_SUITE_END()
