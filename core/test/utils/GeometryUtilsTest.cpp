#include "meshing/Polygon.hpp"
#include "utils/GeometryUtils.hpp"

#include <boost/test/unit_test.hpp>

using namespace utymap;
using namespace utymap::meshing;
using namespace utymap::utils;

BOOST_AUTO_TEST_SUITE(Utils_GeometryUtils)

BOOST_AUTO_TEST_CASE(GivenCoordinates_WhenGetCircle_ThenReturnsCorrectCircle)
{
    std::vector<GeoCoordinate> coordinates =
    {
        { 0, 0 }, { 4, 0 }, { 4, 4 }, { 0, 4 }, { 0, 0 }
    };
    GeoCoordinate center;
    double radius;

    getCircle(coordinates, center, radius);

    BOOST_CHECK_EQUAL(2, center.latitude);
    BOOST_CHECK_EQUAL(2, center.longitude);
    BOOST_CHECK_EQUAL(2, radius);
}

BOOST_AUTO_TEST_CASE(GivenRectangle_WhenGetCircle_ThenReturnsCorrectCircle)
{
    Polygon polygon(1, 0);
    polygon.addContour({ { 0, 0 }, { 4, 0 }, { 4, 4 }, { 0, 4 }, { 0, 0 }});
    Vector2 center;
    double radius;

    getCircle(polygon.rectangle, center, radius);

    BOOST_CHECK_EQUAL(2, center.x);
    BOOST_CHECK_EQUAL(2, center.y);
    BOOST_CHECK_EQUAL(2, radius);
}

BOOST_AUTO_TEST_SUITE_END()