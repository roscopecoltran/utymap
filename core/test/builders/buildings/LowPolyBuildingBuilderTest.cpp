#include "GeoCoordinate.hpp"
#include "builders/BuilderContext.hpp"
#include "builders/buildings/LowPolyBuildingBuilder.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"

#include <boost/test/unit_test.hpp>

#include "test_utils/DependencyProvider.hpp"
#include "test_utils/ElementUtils.hpp"

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::meshing;

namespace {
    const std::string stylesheet = "area|z1[building=yes] { height:10; roof-color:gradient(red); facade-color:gradient(blue); }";
}

struct Builders_Buildings_LowPolyBuildingsBuilderFixture
{
    DependencyProvider dependencyProvider;
};

BOOST_FIXTURE_TEST_SUITE(Builders_Buildings_LowPolyBuildingsBuilder, Builders_Buildings_LowPolyBuildingsBuilderFixture)

BOOST_AUTO_TEST_CASE(GivenRectangle_WhenBuilds_ThenBuildsMesh)
{
    QuadKey quadKey{ 1, 1, 0 };
    bool isCalled = false;
    BuilderContext context(quadKey,
        *dependencyProvider.getStyleProvider(stylesheet),
        *dependencyProvider.getStringTable(),
        *dependencyProvider.getElevationProvider(),
        [&](const Mesh& mesh) {
        isCalled = true;
            BOOST_CHECK_GT(mesh.vertices.size(), 0);
            BOOST_CHECK_GT(mesh.triangles.size(), 0);
            BOOST_CHECK_GT(mesh.colors.size(), 0);
        },
        nullptr);
    Area building = ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(), { { "building", "yes" } },
    { { 0, 0 }, { 0, 10 }, { 10, 10 }, { 10, 0 } });
    LowPolyBuildingBuilder builder(context);

    builder.visitArea(building);

    BOOST_CHECK(isCalled);
}

BOOST_AUTO_TEST_SUITE_END()
