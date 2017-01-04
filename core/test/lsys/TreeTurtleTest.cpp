#include "entities/Node.hpp"
#include "lsys/turtles/TreeTurtle.hpp"
#include "lsys/LSystem.hpp"
#include "lsys/Rules.hpp"

#include <boost/test/unit_test.hpp>

#include "test_utils/DependencyProvider.hpp"
#include "test_utils/ElementUtils.hpp"

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::mapcss;
using namespace utymap::math;
using namespace utymap::lsys;

using namespace utymap::tests;

namespace {
    std::shared_ptr<MoveForwardRule> move() { return std::make_shared<MoveForwardRule>(); }
    std::shared_ptr<WordRule> foliage() { return std::make_shared<WordRule>("foliage"); }

    const std::string stylesheet = "node|z1[natural=tree] { color:gradient(red);}";
    struct Lsys_Turtles_TreeTurtlesFixture
    {
        Lsys_Turtles_TreeTurtlesFixture() :
            dependencyProvider(),
            mesh(""),
            gradient(),
            style(dependencyProvider.getStyleProvider(stylesheet)
            ->forElement(ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 0, { { "natural", "tree" } }), 1)),
            builderContext(
            QuadKey{ 1, 1, 1 },
            *dependencyProvider.getStyleProvider(stylesheet),
            *dependencyProvider.getStringTable(),
            *dependencyProvider.getElevationProvider(),
            [](const Mesh&) {},
            [](const Element&) {}),
            meshContext(mesh, style, gradient, TextureRegion())
        {
        }

        DependencyProvider dependencyProvider;
        Mesh mesh;
        ColorGradient gradient;
        Style style;
        BuilderContext builderContext;
        MeshContext meshContext;
    };
}

BOOST_FIXTURE_TEST_SUITE(Lsys_Turtles_TreeTurtle, Lsys_Turtles_TreeTurtlesFixture)

BOOST_AUTO_TEST_CASE(GivenSimpleLSystem_Run_BuildsNonEmptyMesh)
{
    TreeTurtle turtle(builderContext, meshContext);
    LSystem lsystem;
    lsystem.generations = 1;
    lsystem.axiom.push_back(move());
    lsystem.productions[move()].push_back(std::make_pair<double, LSystem::Rules>(1, { move(), foliage() }));

    turtle.run(lsystem);

    // TODO
   /* BOOST_CHECK_GT(mesh.vertices.size(), 0);
    BOOST_CHECK_GT(mesh.triangles.size(), 0);
    BOOST_CHECK_GT(mesh.colors.size(), 0);*/
}

BOOST_AUTO_TEST_SUITE_END()
