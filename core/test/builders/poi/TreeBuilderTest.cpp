#include "QuadKey.hpp"
#include "builders/poi/TreeBuilder.hpp"
#include "entities/Node.hpp"

#include <boost/test/unit_test.hpp>
#include "test_utils/DependencyProvider.hpp"
#include "test_utils/ElementUtils.hpp"

#include <cstdio>

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::heightmap;
using namespace utymap::index;
using namespace utymap::mapcss;
using namespace utymap::meshing;

namespace {
    const std::string stylesheet = "node|z16[natural=tree], way|z16[natural=tree_row] { "
                                        "foliage-color: gradient(green);"
                                        "trunk-color:gradient(red); foliage-radius:2.5m;"
                                        "trunk-radius:0.2m; trunk-height:4m;"
                                        "tree-step: 3m; }";

    struct Builders_Poi_TreeBuilderFixture
    {
        Builders_Poi_TreeBuilderFixture() :
            dependencyProvider(),
            context(
                QuadKey(16, 35204, 21494),
                *dependencyProvider.getStyleProvider(stylesheet),
                *dependencyProvider.getStringTable(),
                *dependencyProvider.getElevationProvider(),
                [&](const Mesh& mesh) {
                isCalled = true;
                BOOST_CHECK_GT(mesh.vertices.size(), 0);
                BOOST_CHECK_GT(mesh.triangles.size(), 0);
                BOOST_CHECK_GT(mesh.colors.size(), 0);
            }, nullptr),
            isCalled(false)
        {
        }

        DependencyProvider dependencyProvider;
        BuilderContext context;
        bool isCalled;
    };
}

BOOST_FIXTURE_TEST_SUITE(Builders_Poi_TreeBuilder, Builders_Poi_TreeBuilderFixture)

BOOST_AUTO_TEST_CASE(GivenTree_WhenVisitNode_ThenMeshIsBuilt)
{
    Node tree = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 0,{ { "natural", "tree" } });
    tree.coordinate = GeoCoordinate(52.5137977, 13.3818357);
    TreeBuilder builder(context);

    builder.visitNode(tree);

    BOOST_CHECK(isCalled);
}

BOOST_AUTO_TEST_CASE(GivenTreeRow_WhenVisitWay_ThenMeshIsBuilt)
{
    Way treeRow = ElementUtils::createElement<Way>(*dependencyProvider.getStringTable(), 0, 
        { { "natural", "tree_row" } }, 
        { { 52.5137977, 13.3818357 }, { 52.5130465, 13.3822282 }, { 52.5129842, 13.3819065 } });
    TreeBuilder builder(context);

    builder.visitWay(treeRow);

    BOOST_CHECK(isCalled);
}

BOOST_AUTO_TEST_SUITE_END()