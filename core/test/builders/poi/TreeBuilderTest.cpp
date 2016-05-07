#include "QuadKey.hpp"
#include "builders/poi/TreeBuilder.hpp"
#include "entities/Node.hpp"
#include "heightmap/FlatElevationProvider.hpp"

#include <boost/test/unit_test.hpp>

#include "test_utils/ElementUtils.hpp"
#include "test_utils/MapCssUtils.hpp"   
#include <cstdio>

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::heightmap;
using namespace utymap::index;
using namespace utymap::mapcss;
using namespace utymap::meshing;

struct Builders_Poi_TreeBuilderFixture
{
    Builders_Poi_TreeBuilderFixture() :
        stringTable(new StringTable("")),
        styleProvider(MapCssUtils::createStyleProviderFromString(*stringTable,
        "node|z16[natural=tree] { foliage-color: gradient(green); trunk-color:gradient(red); foliage-radius:2.5m; trunk-radius:0.2m; trunk-height:4m; }")),
        eleProvider()
    {
    }

    ~Builders_Poi_TreeBuilderFixture()
    {
        std::remove("string.idx");
        std::remove("string.dat");
    }

    FlatElevationProvider eleProvider;
    std::shared_ptr<StringTable> stringTable;
    std::shared_ptr<StyleProvider> styleProvider;
};

BOOST_FIXTURE_TEST_SUITE(Builders_Poi_TreeBuilder, Builders_Poi_TreeBuilderFixture)

BOOST_AUTO_TEST_CASE(GivenTree_WhenVisitNode_ThenMeshIsBuilt)
{
    QuadKey quadKey{ 16, 0, 0 };
    bool isCalled = false;
    BuilderContext context(quadKey, *styleProvider, *stringTable, eleProvider, [&](const Mesh& mesh) {
        isCalled = true;
        BOOST_CHECK_GT(mesh.vertices.size(), 0);
        BOOST_CHECK_GT(mesh.triangles.size(), 0);
        BOOST_CHECK_GT(mesh.colors.size(), 0);
    }, nullptr);

    Node tree = ElementUtils::createElement<Node>(*stringTable, { { "natural", "tree" } });
    tree.coordinate = GeoCoordinate(52, 13);
    TreeBuilder builder(context);

    builder.visitNode(tree);

    BOOST_CHECK(isCalled);
}

BOOST_AUTO_TEST_SUITE_END()