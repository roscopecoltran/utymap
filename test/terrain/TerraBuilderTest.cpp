#include "terrain/MeshRegion.hpp"
#include "terrain/TerraBuilder.hpp"

#include <boost/test/unit_test.hpp>

using namespace utymap::meshing;
using namespace utymap::terrain;

const double Precision = 0.1e-7;

struct Terrain_TerraBuilderFixture
{
    Terrain_TerraBuilderFixture()
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

    Rectangle<double> clipRect;
};

BOOST_FIXTURE_TEST_SUITE(Terrain_TerraBuilder, Terrain_TerraBuilderFixture)

BOOST_AUTO_TEST_CASE(GivenLargeWater_WhenBuild_ThenMeshIsNotEmpty)
{
    MeshRegion region;
    region.points.push_back(Point<double>(0, 0));
    region.points.push_back(Point<double>(20, 0));
    region.points.push_back(Point<double>(20, 20));
    region.points.push_back(Point<double>(0, 20));
    TerraBuilder builder;
    builder.addWater(region);

    Mesh<double> mesh = builder.build(clipRect, 0);

    BOOST_CHECK_GT(mesh.vertices.size(), 0);
    BOOST_CHECK_GT(mesh.triangles.size(), 0);
}

BOOST_AUTO_TEST_SUITE_END()

