#include "heightmap/ElevationProvider.hpp"
#include "clipper/clipper.hpp"
#include "meshing/Polygon.hpp"
#include "meshing/MeshBuilder.hpp"
#include "meshing/MeshTypes.hpp"
#include "terrain/LineGridSplitter.hpp"

#include <boost/test/unit_test.hpp>

using namespace ClipperLib;
using namespace utymap::heightmap;
using namespace utymap::meshing;
using namespace utymap::terrain;

typedef Point<double> DPoint;

class TestElevationProvider: public ElevationProvider<double>
{
public:
    double getElevation(double x, double y) { return 0; }
};

struct Meshing_MeshingFixture 
{
    Meshing_MeshingFixture(): eleProvider(), builder(eleProvider)
                                    { BOOST_TEST_MESSAGE("setup fixture"); }
    ~Meshing_MeshingFixture()       { BOOST_TEST_MESSAGE("teardown fixture"); }

    TestElevationProvider eleProvider;
    MeshBuilder builder;
};

BOOST_FIXTURE_TEST_SUITE(Meshing_MeshBuilder, Meshing_MeshingFixture)

BOOST_AUTO_TEST_CASE(GivenPolygon_WhenBuild_RefinesCorrectly)
{
    Polygon<double> polygon(4, 0);
    polygon.addContour(std::vector<DPoint>
    {
        DPoint(0, 0),
        DPoint(10, 0),
        DPoint(10, 10),
        DPoint(0, 10)
    });

    Mesh<double> mesh = builder.build(polygon, MeshBuilder::Options
    {
        /* area=*/ 5,
        /* elevation noise frequency*/ 0,
        /* segmentSplit=*/ 0
    });

    BOOST_CHECK_EQUAL(mesh.vertices.size() / 3, 23);
    BOOST_CHECK_EQUAL(mesh.triangles.size() / 3, 29);
}

BOOST_AUTO_TEST_CASE(GivenPolygonWithHole_WhenBuild_RefinesCorrectly)
{
    Polygon<double> polygon(8, 1);
    polygon.addContour(std::vector<DPoint>
    {
        DPoint(0, 0),
        DPoint(10, 0),
        DPoint(10, 10),
        DPoint(0, 10)
    });
    polygon.addHole(std::vector<DPoint>
    {
        DPoint(3, 3),
        DPoint(6, 3),
        DPoint(6, 6),
        DPoint(3, 6)
    });

    Mesh<double> mesh = builder.build(polygon, MeshBuilder::Options
    { 
        /* area=*/ 1, 
        /* elevation noise frequency*/ 0,
        /* segmentSplit=*/ 0 
    });

    BOOST_CHECK_EQUAL(mesh.vertices.size() / 3, 86);
    BOOST_CHECK_EQUAL(mesh.triangles.size() / 3, 137);
}

BOOST_AUTO_TEST_CASE(GivenPolygonProcessedByGridSplitter_WhenBuild_RefinesCorrectly)
{
    std::vector<Point<double>> contour;
    LineGridSplitter<double> splitter;
    int scale = 10;
    splitter.setRoundDigits(1);
    splitter.setScale(scale);
    std::vector<IntPoint> inputPoints
    {
        IntPoint(0 * scale, 0 * scale),
        IntPoint(10 * scale, 0 * scale),
        IntPoint(10 * scale, 10 * scale),
        IntPoint(0 * scale, 10 * scale)
    };
    int lastItemIndex = inputPoints.size() - 1;
    for (int i = 0; i <= lastItemIndex; i++) {
        IntPoint start = inputPoints[i];
        IntPoint end = inputPoints[i == lastItemIndex ? 0 : i + 1];
        splitter.split(start, end, contour);
    }

    Polygon<double> polygon(contour.size(), 0);
    polygon.addContour(contour);

    Mesh<double> mesh = builder.build(polygon, MeshBuilder::Options
    {
        /* area=*/ 1. / scale,
        /* elevation noise frequency*/ 0,
        /* segmentSplit=*/ 0
    });

    BOOST_CHECK(mesh.vertices.size() > 0);
    BOOST_CHECK(mesh.triangles.size() > 0);
}

BOOST_AUTO_TEST_SUITE_END()
