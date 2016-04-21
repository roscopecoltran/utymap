#include "BoundingBox.hpp"
#include "QuadKey.hpp"
#include "utils/GeoUtils.hpp"

#include <boost/test/unit_test.hpp>

using namespace utymap;
using namespace utymap::utils;

class TileRangeVisitor
{
public:
    TileRangeVisitor() :
        count(0),
        lastQuadKey(),
        lastBoundingBox(GeoCoordinate(0, 0), GeoCoordinate(0, 0))
    {
    }

    void operator()(const QuadKey& quadKey, const BoundingBox& bbox)
    {
        lastQuadKey = quadKey;
        lastBoundingBox = bbox;
        count++;
    }

    int count;
    QuadKey lastQuadKey;
    BoundingBox lastBoundingBox;
};

BOOST_AUTO_TEST_SUITE(Index_GeoUtils)

const double Precision = 0.1e-7;
const double TestLatitude = 52.53171;
const double TestLongitude = 13.38730;

BOOST_AUTO_TEST_CASE( GivenTestLocationAtFirstLod_WhenGetQuadKey_ThenReturnValidQuadKey )
{
    QuadKey quadKey = GeoUtils::latLonToQuadKey(GeoCoordinate(TestLatitude, TestLongitude), 1);

    BOOST_CHECK_EQUAL(quadKey.levelOfDetail, 1);
    BOOST_CHECK_EQUAL(quadKey.tileX, 1);
    BOOST_CHECK_EQUAL(quadKey.tileY, 0);
}

BOOST_AUTO_TEST_CASE(GivenTestLocationAtNineLod_WhenGetQuadKey_ThenReturnValidQuadKey)
{
    QuadKey quadKey = GeoUtils::latLonToQuadKey(GeoCoordinate(TestLatitude, TestLongitude), 9);

    BOOST_CHECK_EQUAL(quadKey.levelOfDetail, 9);
    BOOST_CHECK_EQUAL(quadKey.tileX, 275);
    BOOST_CHECK_EQUAL(quadKey.tileY, 167);
}

BOOST_AUTO_TEST_CASE(GivenTestLocationAtNineteenLod_WhenGetQuadKey_ThenReturnValidQuadKey)
{
    QuadKey quadKey = GeoUtils::latLonToQuadKey(GeoCoordinate(TestLatitude, TestLongitude), 19);

    BOOST_CHECK_EQUAL(quadKey.levelOfDetail, 19);
    BOOST_CHECK_EQUAL(quadKey.tileX, 281640);
    BOOST_CHECK_EQUAL(quadKey.tileY, 171914);
}

BOOST_AUTO_TEST_CASE(GivenQuadKeyAtNineteenLod_WhenGetBoundgingBox_ThenReturnValidBoundingBox)
{
    QuadKey quadKey;
    quadKey.levelOfDetail = 19;
    quadKey.tileX = 281640;
    quadKey.tileY = 171914;

    utymap::BoundingBox boundingBox = GeoUtils::quadKeyToBoundingBox(quadKey);

    BOOST_CHECK_CLOSE(boundingBox.minPoint.latitude, 52.531678559, Precision);
    BOOST_CHECK_CLOSE(boundingBox.maxPoint.latitude, 52.532096259, Precision);
    BOOST_CHECK_CLOSE(boundingBox.minPoint.longitude, 13.386840820, Precision);
    BOOST_CHECK_CLOSE(boundingBox.maxPoint.longitude, 13.387527465, Precision);
}

BOOST_AUTO_TEST_CASE(GivenQuadKeyAtNineteenLod_WhenToString_ThenReturnValidCode)
{
    QuadKey quadKey;
    quadKey.levelOfDetail = 19;
    quadKey.tileX = 281640;
    quadKey.tileY = 171914;

    std::string code = GeoUtils::quadKeyToString(quadKey);

    BOOST_CHECK_EQUAL("1202102332220103020", code);
}

BOOST_AUTO_TEST_CASE(GivenBboxAtLodOne_WhenVisitTileRange_VisitsOneTile)
{
    BoundingBox bbox(GeoCoordinate(1, 1), GeoCoordinate(2, 2));
    TileRangeVisitor visitor;

    GeoUtils::visitTileRange(bbox, 1, visitor);

    BOOST_CHECK_EQUAL(1, visitor.count);
    BOOST_CHECK_EQUAL(1, visitor.lastQuadKey.tileX);
    BOOST_CHECK_EQUAL(0, visitor.lastQuadKey.tileY);
}

BOOST_AUTO_TEST_CASE(GivenBboxAtLodOne_WhenVisitTileRange_VisitsTwoTiles)
{
    BoundingBox bbox(GeoCoordinate(-1, 1), GeoCoordinate(2, 2));
    TileRangeVisitor visitor;

    GeoUtils::visitTileRange(bbox, 1, visitor);

    BOOST_CHECK_EQUAL(2, visitor.count);
}

BOOST_AUTO_TEST_SUITE_END()
