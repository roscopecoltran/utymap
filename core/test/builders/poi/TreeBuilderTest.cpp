#include "QuadKey.hpp"
#include "builders/poi/TreeBuilder.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Relation.hpp"

#include <boost/test/unit_test.hpp>
#include "test_utils/DependencyProvider.hpp"
#include "test_utils/ElementUtils.hpp"

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::heightmap;
using namespace utymap::index;
using namespace utymap::mapcss;
using namespace utymap::math;
using namespace utymap::lsys;
using namespace utymap::tests;

namespace {
    const std::string stylesheetStr =
        "node|z16[natural=tree], way|z16[natural=tree_row] { "
            "tree-step: 3m;"
            "lsystem: tree;"
            "leaf-color: gradient(green);"
            "leaf-radius: 2.5m;"
            "leaf-texture-index: 0;"
            "leaf-texture-type: tree;"
            "leaf-texture-scale: 50;"
            "trunk-color: gradient(gray);"
            "trunk-radius: 0.3m;"
            "trunk-height: 1.5m;"
            "trunk-texture-index: 0;"
            "trunk-texture-type: background;"
            "trunk-texture-scale: 200;"
        "}";

    struct Builders_Poi_TreeBuilderFixture
    {
        Builders_Poi_TreeBuilderFixture() :
            dependencyProvider(),
            context(
                QuadKey(16, 35204, 21494),
                *dependencyProvider.getStyleProvider(createStyleSheet()),
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

        StyleSheet createStyleSheet() const
        {
            auto stylesheet = utymap::mapcss::MapCssParser().parse(stylesheetStr);
            LSystem lsystem;
            lsystem.axiom.push_back(std::make_shared<MoveForwardRule>());
            stylesheet.lsystems.emplace("tree", lsystem);
            return stylesheet;
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