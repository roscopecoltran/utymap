#include "builders/generators/CylinderGenerator.hpp"
#include "builders/generators/IcoSphereGenerator.hpp"
#include "builders/generators/TreeGenerator.hpp"
#include "entities/Node.hpp"
#include "utils/GradientUtils.hpp"

#include <boost/test/unit_test.hpp>

#include "test_utils/DependencyProvider.hpp"
#include "test_utils/ElementUtils.hpp"

#include <cstdio>

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::heightmap;
using namespace utymap::mapcss;
using namespace utymap::meshing;
using namespace utymap::index;
using namespace utymap::utils;

namespace {
    const std::string stylesheet = "node|z1[natural=tree] { color:gradient(red);}";
    struct Builders_Generators_GeneratorFixture
    {
        Builders_Generators_GeneratorFixture() :
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
            meshContext(mesh, style, gradient)
        {
        }

        DependencyProvider dependencyProvider;
        Mesh mesh;
        Style style;
        ColorGradient gradient;
        BuilderContext builderContext;
        MeshContext meshContext;
    };
}

BOOST_FIXTURE_TEST_SUITE(Builders_Generators_Generator, Builders_Generators_GeneratorFixture)

BOOST_AUTO_TEST_CASE(GivenIcoSphereGeneratorWithSimpleData_WhenGenerate_ThenCanGenerateMesh)
{
    IcoSphereGenerator icoSphereGenerator(builderContext, meshContext);
    icoSphereGenerator
        .setCenter(Vector3(0, 0, 0))
        .setRadius(10)
        .setRecursionLevel(2)
        .isSemiSphere(false)
        .setColorNoiseFreq(0.1)
        .setVertexNoiseFreq(0.5)
        .generate();

    BOOST_CHECK_GT(mesh.vertices.size(), 0);
    BOOST_CHECK_GT(mesh.triangles.size(), 0);
    BOOST_CHECK_GT(mesh.colors.size(), 0);
}

BOOST_AUTO_TEST_CASE(GivenCylinderGeneratorWithSimpleData_WhenGenerate_ThenCanGenerateMesh)
{
    CylinderGenerator cylinderGenerator(builderContext, meshContext);
    cylinderGenerator
            .setCenter(Vector3(0, 0, 0))
            .setHeight(10)
            .setMaxSegmentHeight(5)
            .setRadialSegments(7)
            .setRadius(1)
            .setColorNoiseFreq(0.1)
            .generate();

    BOOST_CHECK_GT(mesh.vertices.size(), 0);
    BOOST_CHECK_GT(mesh.triangles.size(), 0);
    BOOST_CHECK_GT(mesh.colors.size(), 0);
}

BOOST_AUTO_TEST_CASE(GivenTreeGeneratorWithSimpleData_WhenGenerate_ThenCanGenerateMesh)
{
    TreeGenerator treeGenerator(builderContext, meshContext, ColorGradient(), ColorGradient());
    treeGenerator
            .setPosition(Vector3(0, 0, 0))
            .setTrunkHeight(5)
            .setTrunkRadius(0.5)
            .setFoliageRadius(4)
            .setTrunkColorNoiseFreq(0.1)
            .setFoliageColorNoiseFreq(0.1)
            .generate();

    BOOST_CHECK_GT(mesh.vertices.size(), 0);
    BOOST_CHECK_GT(mesh.triangles.size(), 0);
    BOOST_CHECK_GT(mesh.colors.size(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
