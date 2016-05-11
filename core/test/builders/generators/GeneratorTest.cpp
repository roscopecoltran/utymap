#include "builders/generators/CylinderGenerator.hpp"
#include "builders/generators/IcoSphereGenerator.hpp"
#include "builders/generators/TreeGenerator.hpp"
#include "entities/Node.hpp"
#include "heightmap/FlatElevationProvider.hpp"
#include "index/StringTable.hpp"
#include "utils/GradientUtils.hpp"

#include <boost/test/unit_test.hpp>
#include <cstdio>

#include "test_utils/MapCssUtils.hpp"
#include "test_utils/ElementUtils.hpp"

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::heightmap;
using namespace utymap::mapcss;
using namespace utymap::meshing;
using namespace utymap::index;
using namespace utymap::utils;

struct Builders_GeneratorFixture
{
    // TODO simpify setup
    Builders_GeneratorFixture() :
        stringTable(""),
        styleProvider(MapCssUtils::createStyleProviderFromString(stringTable, "node|z1[natural=tree] { color:gradient(red);}")),
        mesh(""),
        style(styleProvider->forElement(ElementUtils::createElement<Node>(stringTable, { { "natural", "tree" } }), 1)),
        builderContext(
            QuadKey { 1, 1, 1 },
            *styleProvider,
            stringTable,
            FlatElevationProvider(),
            [](const Mesh&) {},
            [](const Element&) {}),
        meshContext(mesh, style),

        icoSphereGenerator(builderContext, meshContext, "color"),
        cylinderGenerator(builderContext, meshContext, "color"),
        treeGenerator(builderContext, meshContext, "color", "color")
    {
    }

    ~Builders_GeneratorFixture()
    {
        std::remove("string.idx");
        std::remove("string.dat");
    }

    StringTable stringTable;
    std::shared_ptr<StyleProvider> styleProvider;
    Mesh mesh;
    Style style;
    BuilderContext builderContext;
    MeshContext meshContext;
 
    IcoSphereGenerator icoSphereGenerator;
    CylinderGenerator cylinderGenerator;
    TreeGenerator treeGenerator;
};

BOOST_FIXTURE_TEST_SUITE(Builders_Generators_Generator, Builders_GeneratorFixture)

BOOST_AUTO_TEST_CASE(GivenIcoSphereGeneratorWithSimpleData_WhenGenerate_ThenCanGenerateMesh)
{
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
    cylinderGenerator
            .setCenter(Vector3(0, 0, 0))
            .setHeight(10)
            .setMaxSegmentHeight(5)
            .setRadialSegments(7)
            .setRadius(1)
            .generate();

    BOOST_CHECK_GT(mesh.vertices.size(), 0);
    BOOST_CHECK_GT(mesh.triangles.size(), 0);
    BOOST_CHECK_GT(mesh.colors.size(), 0);
}

BOOST_AUTO_TEST_CASE(GivenTreeGeneratorWithSimpleData_WhenGenerate_ThenCanGenerateMesh)
{
    treeGenerator
            .setPosition(Vector3(0, 0, 0))
            .setTrunkHeight(5)
            .setTrunkRadius(0.5)
            .setFoliageRadius(4)
            .generate();

    BOOST_CHECK_GT(mesh.vertices.size(), 0);
    BOOST_CHECK_GT(mesh.triangles.size(), 0);
    BOOST_CHECK_GT(mesh.colors.size(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
