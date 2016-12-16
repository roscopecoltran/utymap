#include "utils/GeometryUtils.hpp"

#include <boost/test/unit_test.hpp>

using namespace utymap;
using namespace utymap::math;
using namespace utymap::utils;

namespace boost { namespace test_tools { namespace tt_detail {
    std::ostream& operator<<(std::ostream& os, const Vector2 &v)
    {
        return os << '(' << v.x << ',' << v.y << ')';
    } 
}}}

static inline std::ostream& operator<<(std::ostream &os, const Vector2 &v)
{
    return os;
}

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
    Vector2 size;

    getCircle(polygon.rectangle, center, size);

    BOOST_CHECK_EQUAL(2, center.x);
    BOOST_CHECK_EQUAL(2, center.y);
    BOOST_CHECK_EQUAL(2, size.x);
    BOOST_CHECK_EQUAL(2, size.y);
}

BOOST_AUTO_TEST_CASE(GiveSegment_WhenGetOffsetLine_ThenReturnsCorrectRectangle)
{
    std::vector<Vector2> expected = { { 0, 1 }, { 0, -1 }, { 10, -1 }, { 10, 1 } };

    auto vertices = getOffsetLine({ 0, 0 }, { 10, 0 }, 1);

    BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), 
        vertices.begin(), vertices.end());
}

BOOST_AUTO_TEST_SUITE_END()