#include "heightmap/ElevationProvider.hpp"
#include "builders/terrain/LineGridSplitter.hpp"
#include "clipper/clipper.hpp"
#include "mapcss/ColorGradient.hpp"
#include "meshing/Polygon.hpp"
#include "meshing/MeshBuilder.hpp"
#include "meshing/MeshTypes.hpp"

#include <boost/test/unit_test.hpp>

using namespace ClipperLib;
using namespace utymap::heightmap;
using namespace utymap::mapcss;
using namespace utymap::meshing;

typedef Point DPoint;

const ColorGradient colorGradient(ColorGradient::GradientData{ { 0, Color(0, 0, 0, 0) } });

class TestElevationProvider: public ElevationProvider
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

BOOST_AUTO_TEST_CASE(GivenPolygon_WhenAddPolygon_RefinesCorrectly)
{
    Mesh mesh("");
    Polygon polygon(4, 0);
    polygon.addContour(std::vector<DPoint>
    {
        DPoint(0, 0),
        DPoint(10, 0),
        DPoint(10, 10),
        DPoint(0, 10)
    });

    builder.addPolygon(mesh, polygon, MeshBuilder::Options
    {
        /* area=*/ 5,
        /* elevation noise frequency*/ 0,
        /* color noise frequency */ 0,
        /* height offset */ 0,
        /* color gradient */ colorGradient
    });

    BOOST_CHECK_EQUAL(mesh.vertices.size() / 3, 23);
    BOOST_CHECK_EQUAL(mesh.triangles.size() / 3, 34);
}

BOOST_AUTO_TEST_CASE(GivenPolygonWithHole_WhenAddPolygon_RefinesCorrectly)
{
    Mesh mesh("");
    Polygon polygon(8, 1);
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

    builder.addPolygon(mesh, polygon, MeshBuilder::Options
    { 
        /* area=*/ 1, 
        /* elevation noise frequency*/ 0,
        /* color noise frequency */ 0,
        /* height offset */ 0,
        /* color gradient */ colorGradient
    });

    BOOST_CHECK_EQUAL(mesh.vertices.size() / 3, 84);
    BOOST_CHECK_EQUAL(mesh.triangles.size() / 3, 141);
}

BOOST_AUTO_TEST_CASE(GivenPolygonProcessedByGridSplitter_WhenAddPolygon_RefinesCorrectly)
{
    std::vector<Point> contour;
    LineGridSplitter splitter;
    int scale = 10;
    splitter.setParams(scale, 1);
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

    Polygon polygon(contour.size(), 0);
    polygon.addContour(contour);

    Mesh mesh("");
    builder.addPolygon(mesh, polygon, MeshBuilder::Options
    {
        /* area=*/ 1. / scale,
        /* elevation noise frequency*/ 0,
        /* color noise frequency */ 0,
        /* height offset */ 0,
        /* color gradient */ colorGradient
    });

    BOOST_CHECK(mesh.vertices.size() > 0);
    BOOST_CHECK(mesh.triangles.size() > 0);
}

BOOST_AUTO_TEST_SUITE_END()
