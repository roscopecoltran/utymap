#include "CommonTypes.hpp"
#include "index/GeoUtils.hpp"

#include <boost/test/unit_test.hpp>

using namespace utymap::index;

BOOST_AUTO_TEST_SUITE(Index_GeoUtils)

const double Precision = 0.1e-7;
const double TestLatitude = 52.53171;
const double TestLongitude = 13.38730;

BOOST_AUTO_TEST_CASE( GivenTestLocationAtFirstLod_WhenGetQuadKey_ThenReturnValidQuadKey )
{
    utymap::QuadKey quadKey;

    GeoUtils::latLonToQuadKey(TestLatitude, TestLongitude, 1, quadKey);

    BOOST_CHECK_EQUAL(quadKey.levelOfDetail, 1);
    BOOST_CHECK_EQUAL(quadKey.tileX, 1);
    BOOST_CHECK_EQUAL(quadKey.tileY, 0);
}

BOOST_AUTO_TEST_CASE(GivenTestLocationAtNineLod_WhenGetQuadKey_ThenReturnValidQuadKey)
{
    utymap::QuadKey quadKey;

    GeoUtils::latLonToQuadKey(TestLatitude, TestLongitude, 9, quadKey);

    BOOST_CHECK_EQUAL(quadKey.levelOfDetail, 9);
    BOOST_CHECK_EQUAL(quadKey.tileX, 275);
    BOOST_CHECK_EQUAL(quadKey.tileY, 167);
}

BOOST_AUTO_TEST_CASE(GivenTestLocationAtLastLod_WhenGetQuadKey_ThenReturnValidQuadKey)
{
    utymap::QuadKey quadKey;

    GeoUtils::latLonToQuadKey(TestLatitude, TestLongitude, 19, quadKey);

    BOOST_CHECK_EQUAL(quadKey.levelOfDetail, 19);
    BOOST_CHECK_EQUAL(quadKey.tileX, 281640);
    BOOST_CHECK_EQUAL(quadKey.tileY, 171914);
}

BOOST_AUTO_TEST_CASE(GivenQuadKeyAtNineLod_WhenGetBoundgingBox_ThenReturnValidBoundingBox)
{
    utymap::BoundingBox boundingBox;
    utymap::QuadKey quadKey;
    quadKey.levelOfDetail = 19;
    quadKey.tileX = 281640;
    quadKey.tileY = 171914;

    GeoUtils::quadKeyToBoundingBox(quadKey, boundingBox);

    BOOST_CHECK_CLOSE(boundingBox.minLatitude, 52.531678559, Precision);
    BOOST_CHECK_CLOSE(boundingBox.maxLatitude, 52.532096259, Precision);
    BOOST_CHECK_CLOSE(boundingBox.minLongitude, 13.386840820, Precision);
    BOOST_CHECK_CLOSE(boundingBox.maxLongitude, 13.387527465, Precision);
}

BOOST_AUTO_TEST_CASE(GivenQuadKeyAtNineLod_WhenGetCode_ThenReturnValidCode)
{
    std::string code;
    utymap::QuadKey quadKey;
    quadKey.levelOfDetail = 19;
    quadKey.tileX = 281640;
    quadKey.tileY = 171914;

    GeoUtils::quadKeyToCode(quadKey, code);

    BOOST_CHECK_EQUAL("1202102332220103020", code);
}

BOOST_AUTO_TEST_SUITE_END()
