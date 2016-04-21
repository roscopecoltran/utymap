#include "QuadKey.hpp"
#include "builders/BuilderContext.hpp"
#include "builders/terrain/TerraBuilder.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "heightmap/FlatElevationProvider.hpp"

#include <boost/test/unit_test.hpp>

#include "test_utils/ElementUtils.hpp"
#include "test_utils/MapCssUtils.hpp"

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::heightmap;
using namespace utymap::index;
using namespace utymap::mapcss;
using namespace utymap::meshing;

const char* StyleSheetString =
"canvas|z1 { grid-cell-size: 1%; layer-priority: water; ele-noise-freq: 0.05; color-noise-freq: 0.1; color:gradient(red); max-area: 5%;"
            "water-ele-noise-freq: 0.05; water-color-noise-freq: 0.1; water-color:gradient(red);  water-max-area: 5%;}"
"area|z1[natural=water] { builders:terrain; terrain-layer:water; }";

struct Builders_Terrain_TerraBuilderFixture
{
    Builders_Terrain_TerraBuilderFixture() :
        stringTablePtr(new StringTable("")),
        styleProviderPtr(MapCssUtils::createStyleProviderFromString(*stringTablePtr, StyleSheetString)),
        eleProvider(),
        builderPtr(nullptr)
    {
    }

    ~Builders_Terrain_TerraBuilderFixture()
    {
        delete builderPtr;
        delete styleProviderPtr;
        delete stringTablePtr;

        std::remove("string.idx");
        std::remove("string.dat");
    }

    FlatElevationProvider eleProvider;
    StringTable* stringTablePtr;
    StyleProvider* styleProviderPtr;
    TerraBuilder* builderPtr;
};

BOOST_FIXTURE_TEST_SUITE(Builders_Terrain_TerraBuilder, Builders_Terrain_TerraBuilderFixture)

BOOST_AUTO_TEST_CASE(GivenLargeWater_WhenComplete_ThenMeshIsNotEmpty)
{
    bool isCalled = false;
    BuilderContext context(QuadKey{ 1, 0, 0 }, *styleProviderPtr, *stringTablePtr, eleProvider, 
        [&](const Mesh& mesh) {
            isCalled = true;
            BOOST_CHECK_GT(mesh.vertices.size(), 0);
            BOOST_CHECK_GT(mesh.triangles.size(), 0);
    }, nullptr);
    builderPtr = new TerraBuilder(context);
    ElementUtils::createElement<Area>(*stringTablePtr, 
        { { "natural", "water" } },
        { { 0, 0 }, { 20, 0 }, { 20, 20 }, { 0, 20 } }).accept(*builderPtr);

    builderPtr->complete();

    BOOST_CHECK(isCalled);
}

BOOST_AUTO_TEST_SUITE_END()
