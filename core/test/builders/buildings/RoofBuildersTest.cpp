#include "GeoCoordinate.hpp"
#include "builders/BuilderContext.hpp"
#include "builders/buildings/roofs/DomeRoofBuilder.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"

#include <boost/test/unit_test.hpp>

#include "test_utils/DependencyProvider.hpp"
#include "test_utils/ElementUtils.hpp"

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::mapcss;
using namespace utymap::meshing;

struct Builders_Buildings_RoofBuildersFixture
{
    DependencyProvider dependencyProvider;
};

BOOST_FIXTURE_TEST_SUITE(Builders_Buildings_RoofBuilders, Builders_Buildings_RoofBuildersFixture)

BOOST_AUTO_TEST_CASE(GivenDome_WhenBuilds_ThenBuildsMesh)
{
    std::string stylesheet = "area|z16[building=yes] { roof-color:gradient(red); }";
    Area building = ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(), 
    { { "building", "yes" }, {"roof:shape", "dome" } }, { });
    auto builderContext = dependencyProvider.createBuilderContext(QuadKey(16, 1, 0), stylesheet);
    Style style = dependencyProvider.getStyleProvider(stylesheet)->forElement(building, 16);
    Mesh mesh("");
    MeshContext meshContext(mesh, style);
    Polygon polygon(0, 0);
    polygon.addContour({ { 0, 0 }, { 0, 10 }, { 10, 10 }, { 10, 0 } });
    DomeRoofBuilder builder(*builderContext, meshContext);
    builder.setMinHeight(10);

    builder.build(polygon);

    BOOST_CHECK_GT(mesh.vertices.size(), 0);
    BOOST_CHECK_GT(mesh.triangles.size(), 0);
    BOOST_CHECK_GT(mesh.colors.size(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
