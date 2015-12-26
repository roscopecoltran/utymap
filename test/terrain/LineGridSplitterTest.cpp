#include "terrain/LineGridSplitter.hpp"
#include "terrain/MeshRegion.hpp"

#include <boost/test/unit_test.hpp>

using namespace ClipperLib;
using namespace utymap::meshing;
using namespace utymap::terrain;

typedef std::vector<Point<double>> DoublePoints;
const double Precision = 0.1e-9;

BOOST_AUTO_TEST_SUITE(Terrain_LineGridSplitter)

BOOST_AUTO_TEST_CASE(GivenHorizontal_WhenSplitWithIntStep_CanSplit)
{
    IntPoint start(0, 0);
    IntPoint end(10, 0);
    LineGridSplitter<double> splitter;
    DoublePoints result;

    splitter.split(start, end, result);

    for (int i = 0; i <= 10; ++i) {
        BOOST_CHECK_CLOSE(i, result[i].x, Precision);
        BOOST_CHECK_CLOSE(0, result[i].y, Precision);
    }
}

BOOST_AUTO_TEST_CASE(GivenVertical_WhenSplitWithIntStep_CanSplit)
{
    IntPoint start(0, 0);
    IntPoint end(0, 10);
    LineGridSplitter<double> splitter;
    DoublePoints result;

    splitter.split(start, end, result);

    for (int i = 0; i <= 10; ++i) {
        BOOST_CHECK_CLOSE(0, result[i].x, Precision);
        BOOST_CHECK_CLOSE(i, result[i].y, Precision);
    }
}

BOOST_AUTO_TEST_CASE(Given45Angle_WhenSplitWithIntStep_CanSplit)
{
    IntPoint start(0, 0);
    IntPoint end(-10, 10);
    LineGridSplitter<double> splitter;
    DoublePoints result;

    splitter.split(start, end, result);

    for (int i = 0; i <= 10; ++i) {
        BOOST_CHECK_CLOSE(-i, result[i].x, Precision);
        BOOST_CHECK_CLOSE(i, result[i].y, Precision);
    }
}

BOOST_AUTO_TEST_CASE(Given45Angle_WhenSplitWithHighLoD_CanSplit)
{
    int roundDigits = 1;
    double scale = 10000000;
    LineGridSplitter<double> splitter;
    splitter.setRoundDigits(roundDigits);
    splitter.setScale(scale);
    IntPoint start(0, 0);
    IntPoint end(10, 10);
    DoublePoints result;

    splitter.split(start, end, result);

    double step = 1 / scale;
    for (int i = 0; i <= 10; ++i) {
        BOOST_CHECK_CLOSE(i * step, result[i].x, Precision);
        BOOST_CHECK_CLOSE(i * step, result[i].y, Precision);
    }
}

BOOST_AUTO_TEST_SUITE_END()