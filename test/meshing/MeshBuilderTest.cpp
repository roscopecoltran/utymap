#include "heightmap/ElevationProvider.hpp"
#include "meshing/Polygon.hpp"
#include "meshing/MeshBuilder.hpp"
#include "meshing/MeshTypes.hpp"

#include <boost/test/unit_test.hpp>

using namespace utymap::heightmap;
using namespace utymap::meshing;

typedef Point<double> DoublePoint;

class TestElevationProvider: public ElevationProvider<double>
{
public:
    float getElevation(double x, double y) { return 0; }
};

struct Meshing_MeshingFixture {
    Meshing_MeshingFixture()
        : eleProvider(), builder(eleProvider)
    { BOOST_TEST_MESSAGE("setup fixture"); }
    ~Meshing_MeshingFixture()       { BOOST_TEST_MESSAGE("teardown fixture"); }

    TestElevationProvider eleProvider;
    MeshBuilder builder;
};

BOOST_FIXTURE_TEST_SUITE( Meshing_MeshBuilder, Meshing_MeshingFixture )

BOOST_AUTO_TEST_CASE( GivenPolygonWithHole_WhenBuild_RefinesCorrectly )
{
    Polygon<double> polygon(8, 1);

    polygon.addContour(std::vector <Point<double> >
    {
        DoublePoint(0, 0),
        DoublePoint(10, 0),
        DoublePoint(10, 10),
        DoublePoint(0, 10)
    });

    polygon.addHole(std::vector <Point<double> >
    {
        DoublePoint(3, 3),
        DoublePoint(6, 3),
        DoublePoint(6, 6),
        DoublePoint(3, 6)
    });

    builder.build(polygon, MeshBuilder::Options{ /*area=*/ 4, /* segmentSplit=*/ 0 });
}

BOOST_AUTO_TEST_SUITE_END()
