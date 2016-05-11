#include "GeoCoordinate.hpp"
#include "builders/BuilderContext.hpp"
#include "builders/buildings/LowPolyBuildingBuilder.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "heightmap/FlatElevationProvider.hpp"
#include "test_utils/MapCssUtils.hpp"
#include "test_utils/ElementUtils.hpp"

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::heightmap;
using namespace utymap::index;
using namespace utymap::mapcss;
using namespace utymap::meshing;

struct Builders_Buildings_LowPolyBuildingsBuilderFixture
{
    Builders_Buildings_LowPolyBuildingsBuilderFixture() :
        stringTable(""),
        styleProvider(MapCssUtils::createStyleProviderFromString(stringTable,
            "area|z1[building=yes] { height:10; roof-color:gradient(red); facade-color:gradient(blue); }")),
        eleProvider()
    {
    }

    ~Builders_Buildings_LowPolyBuildingsBuilderFixture()
    {
        std::remove("string.idx");
        std::remove("string.dat");
    }

    FlatElevationProvider eleProvider;
    StringTable stringTable;
    std::shared_ptr<StyleProvider> styleProvider;
};


BOOST_FIXTURE_TEST_SUITE(Builders_Buildings_LowPolyBuildingsBuilder, Builders_Buildings_LowPolyBuildingsBuilderFixture)

BOOST_AUTO_TEST_CASE(GivenRectangle_WhenBuilds_ThenBuildsMesh)
{
    QuadKey quadKey{ 1, 1, 0 };
    bool isCalled = false;
    BuilderContext context(quadKey, *styleProvider, stringTable, eleProvider, [&](const Mesh& mesh) {
        isCalled = true;
        BOOST_CHECK_GT(mesh.vertices.size(), 0);
        BOOST_CHECK_GT(mesh.triangles.size(), 0);
        BOOST_CHECK_GT(mesh.colors.size(), 0);
    }, nullptr);
    Area building = ElementUtils::createElement<Area>(stringTable, { { "building", "yes" } },
        { { 0, 0 }, {0, 10}, {10, 10}, {10, 0} });
    LowPolyBuildingBuilder builder(context);

    builder.visitArea(building);

    BOOST_CHECK(isCalled);
}

BOOST_AUTO_TEST_SUITE_END()
