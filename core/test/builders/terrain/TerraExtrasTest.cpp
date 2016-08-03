#include "builders/terrain/TerraExtras.hpp"
#include "entities/Area.hpp"

#include <boost/test/unit_test.hpp>
#include "test_utils/DependencyProvider.hpp"
#include "test_utils/ElementUtils.hpp"

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::mapcss;
using namespace utymap::meshing;

namespace {
    auto colorGradient = std::make_shared<const ColorGradient>(ColorGradient::GradientData{ { 0, Color(0, 0, 0, 0) } });
    const std::string stylesheet = "area|z16[amenity=forest] { "
                                    "foliage-color: gradient(green);"
                                    "trunk-color:gradient(red); foliage-radius:2.5m;"
                                    "trunk-radius:0.2m; trunk-height:4m; }";

    struct Builders_Terrain_TerraExtrasFixture
    {
        Builders_Terrain_TerraExtrasFixture():
            dependencyProvider(),
            builderContext(
                QuadKey(16, 0, 0),
                *dependencyProvider.getStyleProvider(stylesheet),
                *dependencyProvider.getStringTable(),
                *dependencyProvider.getElevationProvider(),
                nullptr,
                nullptr)
        {
        }

        std::shared_ptr<Mesh> generateMesh()
        {
            auto mesh = std::make_shared<Mesh>("area");
            Polygon polygon(4, 0);
            polygon.addContour(std::vector < Vector2 > {{0, 0}, { 10, 0 }, { 10, 10 }, { 0, 10 } });
            MeshBuilder builder(*dependencyProvider.getElevationProvider());
            builder.addPolygon(*mesh, polygon, MeshBuilder::Options(5, 0, 0, 0, colorGradient));
            return mesh;
        }

        Style generateStyle()
        {
            Area area = ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(), 0, 
                { { "amenity", "forest" } },
                { { 0, 0 }, { 10, 0 }, { 10, 10 }, { 0, 10 } });
            return dependencyProvider.getStyleProvider(stylesheet)->forElement(area, 16);
        }

        DependencyProvider dependencyProvider;
        BuilderContext builderContext;
    };   
}

BOOST_FIXTURE_TEST_SUITE(Builders_Terrain_TerraExtras, Builders_Terrain_TerraExtrasFixture)

BOOST_AUTO_TEST_CASE(GivenMesh_WhenAddForest_ThenTreesAreAdded)
{ 
    auto mesh = generateMesh();
    auto style = generateStyle();
    TerraExtras::Context extrasContext(*mesh, style);
    extrasContext.startVertex = 0, extrasContext.startTriangle = 0, 
        extrasContext.startColor = 0;
    std::size_t vertexCount = mesh->vertices.size(),
        triCount = mesh->triangles.size(), 
        colorCount = mesh->colors.size();

    TerraExtras::addForest(builderContext, extrasContext);

    BOOST_CHECK_GT(mesh->vertices.size(), vertexCount);
    BOOST_CHECK_GT(mesh->triangles.size(), triCount);
    BOOST_CHECK_GT(mesh->colors.size(), colorCount);
}

BOOST_AUTO_TEST_SUITE_END()