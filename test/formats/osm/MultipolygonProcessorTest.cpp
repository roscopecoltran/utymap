#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "formats/osm/MultipolygonProcessor.hpp"
#include "index/StringTable.hpp"
#include "test_utils/ElementUtils.hpp"

#include <boost/test/unit_test.hpp>

#include <initializer_list>
#include <tuple>

using namespace utymap::entities;
using namespace utymap::formats;
using namespace utymap::index;

struct Formats_Osm_MultipolygonProcessorFixture
{
    Formats_Osm_MultipolygonProcessorFixture() : stringTablePtr(new StringTable("")), areaMap(), wayMap()
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
    BOOST_CHECK_EQUAL(5, reinterpret_cast<const Area&>(*relation.elements[1]).coordinates.size());
}

BOOST_AUTO_TEST_SUITE_END()