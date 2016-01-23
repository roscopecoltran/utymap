#include "builders/TerraBuilder.hpp"
#include "heightmap/ElevationProvider.hpp"

#include <boost/test/unit_test.hpp>

using namespace utymap::builders;
using namespace utymap::heightmap;
using namespace utymap::meshing;

const double Precision = 0.1e-7;

class TestElevationProvider : public ElevationProvider<double>
{
public:
    double getElevation(double x, double y) { return 0; }
};

struct Terrain_TerraBuilderFixture
{
    Terrain_TerraBuilderFixture() //:
       // builder(eleProvider)
    {
        BOOST_TEST_MESSAGE("setup fixture");
        Contour<double> contour;
        contour.push_back(Point<double>(0, 0));
        contour.push_back(Point<double>(10, 0));
        contour.push_back(Point<double>(10, 10));
        contour.push_back(Point<double>(0, 10));
        clipRect.expand(contour);
    }

    ~Terrain_TerraBuilderFixture()
    {
        BOOST_TEST_MESSAGE("teardown fixture");
    }

    TestElevationProvider eleProvider;
    //TerraBuilder builder;
    Rectangle<double> clipRect;
};

BOOST_FIXTURE_TEST_SUITE(Terrain_TerraBuilder, Terrain_TerraBuilderFixture)

BOOST_AUTO_TEST_CASE(GivenLargeWater_WhenBuild_ThenMeshIsNotEmpty)
{
    /*MeshRegion region;
    region.points.push_back(Point<double>(0, 0));
    region.points.push_back(Point<double>(20, 0));
    region.points.push_back(Point<double>(20, 20));
    region.points.push_back(Point<double>(0, 20));
    builder.addWater(region);

    Mesh<double> mesh = builder.build(clipRect, 1);

    BOOST_CHECK_GT(mesh.vertices.size(), 0);
    BOOST_CHECK_GT(mesh.triangles.size(), 0);*/
}

BOOST_AUTO_TEST_SUITE_END()
