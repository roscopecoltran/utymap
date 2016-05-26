#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "formats/osm/MultipolygonProcessor.hpp"
#include "test_utils/ElementUtils.hpp"

#include <boost/test/unit_test.hpp>
#include "test_utils/DependencyProvider.hpp"

#include <cstdio>

using namespace utymap;
using namespace utymap::entities;
using namespace utymap::formats;
using namespace utymap::index;

struct Formats_Osm_MultipolygonProcessorFixture
{
    Formats_Osm_MultipolygonProcessorFixture()
            : dependencyProvider(), context()
    {
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

    DependencyProvider dependencyProvider;
    OsmDataContext context;
};

BOOST_FIXTURE_TEST_SUITE(Formats_Osm_MultipolygonProcessor, Formats_Osm_MultipolygonProcessorFixture)

BOOST_AUTO_TEST_CASE(GivenOneOuterOneInnerAllClosed_WhenProcess_ThenReturnCorrectResult)
{
    RelationMembers relationMembers = createRelationMembers({
        std::make_tuple(1, "way", "outer"),
        std::make_tuple(2, "way", "inner")
    });
    Tags tags = { utymap::formats::Tag{ "type", "multipolygon" }, utymap::formats::Tag{ "tag", "tags" } };
    context.areaMap[1] = std::shared_ptr<Area>(new Area(ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(), {},
                    { { 0, 0 }, { 3, 5 }, {7, 3}, {8, -1}, {3, -4}, {0, 0} })));
    context.areaMap[2] = std::shared_ptr<Area>(new Area(ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(), {},
                    { { 2, 0 }, { 3, 2 }, { 5, 1 }, { 4, -1 }, { 2, 0 } })));
    MultipolygonProcessor processor(0, relationMembers, tags, *dependencyProvider.getStringTable(), context);

    processor.process();

    auto relation = context.relationMap[0];
    BOOST_CHECK_EQUAL(2, relation->elements.size());

    BOOST_CHECK_EQUAL(6, reinterpret_cast<const Area&>(*relation->elements[0]).coordinates.size());
    BOOST_CHECK(context.areaMap[1]->coordinates == reinterpret_cast<const Area&>(*relation->elements[0]).coordinates);

    BOOST_CHECK_EQUAL(5, reinterpret_cast<const Area&>(*relation->elements[1]).coordinates.size());
    std::reverse(context.areaMap[2]->coordinates.begin(), context.areaMap[2]->coordinates.end());
    BOOST_CHECK(context.areaMap[2]->coordinates == reinterpret_cast<const Area&>(*relation->elements[1]).coordinates);
}

BOOST_AUTO_TEST_CASE(GivenOneOuterTwoInnerAllClosed_WhenProcess_ThenReturnCorrectResult)
{
    RelationMembers relationMembers = createRelationMembers({
        std::make_tuple(1, "way", "outer"),
        std::make_tuple(2, "way", "inner"),
        std::make_tuple(3, "way", "inner"),
    });
    Tags tags = { utymap::formats::Tag{ "type", "multipolygon" }, utymap::formats::Tag{ "tag", "tags" } };
    context.areaMap[1] = std::shared_ptr<Area>(new Area(ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(), {},
                    { { 0, 0 }, { 3, 5 }, { 7, 3 }, { 8, -1 }, { 3, -4 }, { 0, 0 } })));
    context.areaMap[2] = std::shared_ptr<Area>(new Area(ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(), {},
                    { { 2, 1 }, { 3, 3 }, { 5, 2 }, { 4, 0 }, { 2, 1 } })));
    context.areaMap[3] = std::shared_ptr<Area>(new Area(ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(), {},
                    { { 3, -1 }, { 5, -1 }, { 3, -3 }, { 2, -2 }, { 3, -1 } })));
    MultipolygonProcessor processor(0, relationMembers, tags, *dependencyProvider.getStringTable(), context);

    processor.process();

    auto relation = context.relationMap[0];
    BOOST_CHECK_EQUAL(3, relation->elements.size());

    BOOST_CHECK_EQUAL(6, reinterpret_cast<const Area&>(*relation->elements[0]).coordinates.size());
    BOOST_CHECK(context.areaMap[1]->coordinates == reinterpret_cast<const Area&>(*relation->elements[0]).coordinates);

    BOOST_CHECK_EQUAL(5, reinterpret_cast<const Area&>(*relation->elements[1]).coordinates.size());
    std::reverse(context.areaMap[2]->coordinates.begin(), context.areaMap[2]->coordinates.end());
    BOOST_CHECK(context.areaMap[2]->coordinates == reinterpret_cast<const Area&>(*relation->elements[1]).coordinates);

    BOOST_CHECK_EQUAL(5, reinterpret_cast<const Area&>(*relation->elements[2]).coordinates.size());
    std::reverse(context.areaMap[3]->coordinates.begin(), context.areaMap[3]->coordinates.end());
    BOOST_CHECK(context.areaMap[3]->coordinates == reinterpret_cast<const Area&>(*relation->elements[2]).coordinates);
}

BOOST_AUTO_TEST_CASE(GivenOneOuterNonClosed_WhenProcess_ThenReturnCorrectResult)
{
    RelationMembers relationMembers = createRelationMembers({
        std::make_tuple(1, "way", "outer"),
        std::make_tuple(2, "way", "outer")
    });
    Tags tags = { utymap::formats::Tag{ "type", "multipolygon" }, utymap::formats::Tag{ "tag", "tags" } };
    context.wayMap[1] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), {},
                    { { 0, 0 }, { 3, 5 }, { 7, 3 } })));
    context.wayMap[2] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), {},
                    { { 7, 3 }, { 8, -1 }, { 3, -4 }, { 0, 0 } })));
    MultipolygonProcessor processor(0, relationMembers, tags, *dependencyProvider.getStringTable(), context);

    processor.process();

    auto relation = context.relationMap[0];
    BOOST_CHECK_EQUAL(1, relation->elements.size());
    BOOST_CHECK_EQUAL(6, reinterpret_cast<const Area&>(*relation->elements[0]).coordinates.size());
    std::vector<GeoCoordinate> expected = { { 7, 3 }, { 8, -1 }, { 3, -4 }, { 0, 0 }, { 3, 5 }, { 7, 3 } };
    BOOST_CHECK(expected == reinterpret_cast<const Area&>(*relation->elements[0]).coordinates);
}

BOOST_AUTO_TEST_CASE(GivenTwoOuterClosed_WhenProcess_ThenReturnCorrectResult)
{
    RelationMembers relationMembers = createRelationMembers({
        std::make_tuple(1, "way", "outer"),
        std::make_tuple(2, "way", "outer")
    });
    Tags tags = { utymap::formats::Tag{ "type", "multipolygon" }, utymap::formats::Tag{ "tag", "tags" } };
    context.areaMap[1] = std::shared_ptr<Area>(new Area(ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(), {},
                    { { 0, 0 }, { 3, 5 }, { 7, 3 }, { 8, -1 }, { 3, -4 }, { 0, 0 } })));
    context.areaMap[2] = std::shared_ptr<Area>(new Area(ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(), {},
                    { { 10, -3 }, { 14, -3 }, { 14, -6 }, { 10, -6 }, { 10, -3 } })));
    MultipolygonProcessor processor(0, relationMembers, tags, *dependencyProvider.getStringTable(), context);

    processor.process();

    auto relation = context.relationMap[0];
    BOOST_CHECK_EQUAL(2, relation->elements.size());
    BOOST_CHECK(context.areaMap[1]->coordinates == reinterpret_cast<const Area&>(*relation->elements[1]).coordinates);
    BOOST_CHECK(context.areaMap[2]->coordinates == reinterpret_cast<const Area&>(*relation->elements[0]).coordinates);
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
    context.wayMap[1] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), {},
                    { { 0, 0 }, { 3, 5 }, { 7, 3 } })));
    context.wayMap[2] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), {},
                    { { 7, 3 }, { 8, -1 }, { 3, -4 }, { 0, 0 } })));
    context.areaMap[3] = std::shared_ptr<Area>(new Area(ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(), {},
                    { { 2, 1 }, { 3, 3 }, { 5, 2 }, { 4, 0 }, { 2, 1 } })));
    context.areaMap[4] = std::shared_ptr<Area>(new Area(ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(), {},
                    { { 3, -1 }, { 5, -1 }, { 3, -3 }, { 2, -2 }, { 3, -1 } })));
    MultipolygonProcessor processor(0, relationMembers, tags, *dependencyProvider.getStringTable(), context);

    processor.process();

    auto relation = context.relationMap[0];
    BOOST_CHECK_EQUAL(3, relation->elements.size());

    BOOST_CHECK_EQUAL(6, reinterpret_cast<const Area&>(*relation->elements[0]).coordinates.size());
    std::vector<GeoCoordinate> expected = { { 7, 3 }, { 8, -1 }, { 3, -4 }, { 0, 0 }, { 3, 5 }, { 7, 3 } };
    BOOST_CHECK(expected == reinterpret_cast<const Area&>(*relation->elements[0]).coordinates);

    BOOST_CHECK_EQUAL(5, reinterpret_cast<const Area&>(*relation->elements[2]).coordinates.size());
    std::reverse(context.areaMap[3]->coordinates.begin(), context.areaMap[3]->coordinates.end());
    BOOST_CHECK(context.areaMap[3]->coordinates == reinterpret_cast<const Area&>(*relation->elements[2]).coordinates);

    BOOST_CHECK_EQUAL(5, reinterpret_cast<const Area&>(*relation->elements[1]).coordinates.size());
    std::reverse(context.areaMap[4]->coordinates.begin(), context.areaMap[4]->coordinates.end());
    BOOST_CHECK(context.areaMap[4]->coordinates == reinterpret_cast<const Area&>(*relation->elements[1]).coordinates);
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
    context.wayMap[1] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), {}, { { 1, 5 }, { 8, 4 } })));
    context.wayMap[2] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), {}, { { 8, 4 }, { 9, -1 } })));
    context.wayMap[3] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), {}, { { 9, -1 }, { 8, -6 }, { 2, -5 } })));
    context.wayMap[4] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), {}, { { 2, -5 }, { 0, -3 }, { 1, 5 } })));

    context.wayMap[5] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), {}, { { 2, 1 }, { 3, 3 }, { 6, 3 } })));
    context.wayMap[6] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), {}, { { 6, 3 }, { 4, 0 }, { 2, 1 } })));
    context.wayMap[7] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), {}, { { 1, -2 }, { 3, -1 } })));
    context.wayMap[8] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), {}, { { 3, -1 }, { 4, -4 } })));
    context.wayMap[9] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), {}, { { 4, -4 }, { 1, -3 } })));
    context.wayMap[10] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), {}, { { 1, -3 }, { 1, -2 } })));
    context.areaMap[11] = std::shared_ptr<Area>(new Area(ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(), {}, { { 6, -3 }, { 7, -1 }, { 8, -4 }, { 6, -3 } })));

    context.wayMap[12] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), {}, { { 10, 5 }, { 14, 5 } })));
    context.wayMap[13] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), {}, { { 14, 5 }, { 14, -1 } })));
    context.wayMap[14] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), {}, { { 14, -1 }, { 10, -1 } })));
    context.wayMap[15] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), {}, { { 10, -1 }, { 10, 5 } })));

    context.wayMap[16] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), {}, { { 11, 4 }, { 13, 4 } })));
    context.wayMap[17] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), {}, { { 13, 4 }, { 13, 0 }, { 11, 0 } })));
    context.wayMap[18] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), {}, { { 11, 0 }, { 12, 2 } })));
    context.wayMap[19] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), {}, { { 12, 2 }, { 11, 4 } })));

    context.areaMap[20] = std::shared_ptr<Area>(new Area(ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(), {}, { { 10, -3 }, { 14, -3 }, { 14, -6 }, { 10, -6 }, { 10, -3 } })));
    MultipolygonProcessor processor(0, relationMembers, tags, *dependencyProvider.getStringTable(), context);

    processor.process();

    auto relation = context.relationMap[0];
    BOOST_CHECK_EQUAL(7, relation->elements.size());
}

// Reproducing crash.
BOOST_AUTO_TEST_CASE(GivenSpecificFourOuter_WhenProcess_ThenDoesNotCrash)
{
    RelationMembers relationMembers = createRelationMembers({
        std::make_tuple(1, "way", "outer"),
        std::make_tuple(2, "way", "outer"),
        std::make_tuple(3, "way", "outer"),
        std::make_tuple(4, "way", "outer")
    });
    Tags tags = { utymap::formats::Tag{ "type", "multipolygon" }, utymap::formats::Tag{ "tag", "tags" } };
    context.wayMap[1] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(),
    {}, { { 55.754873400000001, 37.620234000000004 }, { 55.754922700000002, 37.620224499999999 } })));
    context.wayMap[2] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(),
    {}, { { 55.754873400000001, 37.620234000000004 }, { 55.754846999999998, 37.620192099999997 } })));
    context.wayMap[3] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(),
    {}, { { 55.754846999999998, 37.620192099999997 }, { 55.754846100000002, 37.620103100000001 } })));
    context.wayMap[4] = std::shared_ptr<Way>(new Way(ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(),
    {}, { { 55.754846100000002, 37.620103100000001 }, { 55.754922700000002, 37.620224499999999 } })));
    MultipolygonProcessor processor(0, relationMembers, tags, *dependencyProvider.getStringTable(), context);

    processor.process();

    auto relation = context.relationMap[0];
    BOOST_CHECK_EQUAL(1, relation->elements.size());
    BOOST_CHECK_EQUAL(5, reinterpret_cast<const Area&>(*relation->elements[0]).coordinates.size());
}

BOOST_AUTO_TEST_SUITE_END()
