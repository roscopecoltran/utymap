#include "builders/generators/CylinderGenerator.hpp"
#include "builders/generators/IcoSphereGenerator.hpp"
#include "builders/generators/TreeGenerator.hpp"
#include "heightmap/FlatElevationProvider.hpp"
#include "utils/GradientUtils.hpp"

#include <boost/test/unit_test.hpp>

using namespace utymap::builders;
using namespace utymap::heightmap;
using namespace utymap::mapcss;
using namespace utymap::meshing;
using namespace utymap::utils;

struct Builders_GeneratorFixture
{
    Builders_GeneratorFixture() :
            mesh("IcoSphere"),
            gradient(GradientUtils::parseGradient("gradient(red)")),
            icoSphereGenerator(mesh, MeshBuilder(FlatElevationProvider()), gradient),
            cylinderGenerator(mesh, MeshBuilder(FlatElevationProvider()), gradient),
            treeGenerator(mesh, MeshBuilder(FlatElevationProvider()), gradient, gradient)
    {
    }

    Mesh mesh;
    ColorGradient gradient;
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
