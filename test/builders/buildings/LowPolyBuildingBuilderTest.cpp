#include "GeoCoordinate.hpp"
#include "builders/buildings/LowPolyBuildingBuilder.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "heightmap/ElevationProvider.hpp"
#include "index/StringTable.hpp"
#include "mapcss/StyleProvider.hpp"
#include "test_utils/MapCssUtils.hpp"
#include "test_utils/ElementUtils.hpp"

#include <boost/test/unit_test.hpp>
#include <cstdio>
#include <memory>

using utymap::GeoCoordinate;
using utymap::QuadKey;
using utymap::builders::LowPolyBuildingBuilder;
using utymap::entities::Area;
using utymap::entities::Relation;
using utymap::heightmap::ElevationProvider;
using utymap::index::StringTable;
using utymap::mapcss::StyleProvider;
using utymap::meshing::Mesh;

namespace {

class TestElevationProvider : public ElevationProvider 
{
public:
    double getElevation(double x, double y) { return 0; }
};

}

struct Builders_Buildings_LowPolyBuildingsBuilderFixture
{
    Builders_Buildings_LowPolyBuildingsBuilderFixture() :
        stringTable(new StringTable("")),
        styleProvider(MapCssUtils::createStyleProviderFromString(*stringTable, "area|z-1[building=yes] { height:10;}")),
        eleProvider()
    {
    }

    ~Builders_Buildings_LowPolyBuildingsBuilderFixture()
    {
        std::remove("string.idx");
        std::remove("string.dat");
    }

    TestElevationProvider eleProvider;
    std::shared_ptr<StringTable> stringTable;
    std::shared_ptr<StyleProvider> styleProvider;
};


BOOST_FIXTURE_TEST_SUITE(Builders_Buildings_LowPolyBuildingsBuilder, Builders_Buildings_LowPolyBuildingsBuilderFixture)

BOOST_AUTO_TEST_CASE(GivenRectangle_WhenBuilds_ThenBuildsMesh)
{
    QuadKey quadKey{ 1, 1, 0 };
    Area building = ElementUtils::createElement<Area>(*stringTable, { { "building", "yes" } }, 
        { { 0, 0 }, {10, 0}, {10, 10}, {0, 10} });
    bool isCalled = false;
    LowPolyBuildingBuilder builder(quadKey, *styleProvider, *stringTable, eleProvider, [&](const Mesh& mesh) {
        isCalled = true;
        BOOST_CHECK_GT(mesh.vertices.size(), 0);
        BOOST_CHECK_GT(mesh.triangles.size(), 0);
    });

    builder.visitArea(building);

    BOOST_CHECK(isCalled);
}

BOOST_AUTO_TEST_SUITE_END()