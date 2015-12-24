#include "terrain/LineGridSplitter.hpp"
#include "terrain/MeshRegion.hpp"

#include <boost/test/unit_test.hpp>

using namespace utymap::meshing;
using namespace utymap::terrain;

typedef Point<double> DoublePoint;
typedef std::vector<DoublePoint> DoublePoints;
const double Precision = 0.1e-7;

BOOST_AUTO_TEST_SUITE(Terrain_LineGridSplitter)

BOOST_AUTO_TEST_CASE(GivenHorizontal_WhenSplit_CanSplit)
{
    DoublePoint start(0, 0);
    DoublePoint end(10, 0);
    LineGridSplitter<double> splitter(0);

    DoublePoints result = splitter.split(start, end);

    for (int i = 0; i <= 10; ++i) {
        BOOST_CHECK_CLOSE(i, result[i].x, Precision);
        BOOST_CHECK_CLOSE(0, result[i].y, Precision);
    }
}

BOOST_AUTO_TEST_SUITE_END()