#include "utils/GeometryUtils.hpp"

#include <boost/test/unit_test.hpp>

using namespace utymap;
using namespace utymap::utils;

BOOST_AUTO_TEST_SUITE(Utils_GeometryUtils)

BOOST_AUTO_TEST_CASE(GivenCoordinates_WhenGetCircle_ThenReturnsCorrectCircle)
{
    std::vector<GeoCoordinate> coordinates =
    {
        { 0, 0 }, { 4, 0 }, { 4, 4 }, { 0, 4 }, { 0, 0 },
    };
    GeoCoordinate center;
    double radius;

    getCircle(coordinates, center, radius);

    BOOST_CHECK_EQUAL(2, center.latitude);
    BOOST_CHECK_EQUAL(2, center.longitude);
    BOOST_CHECK_EQUAL(2, radius);
}

BOOST_AUTO_TEST_SUITE_END()