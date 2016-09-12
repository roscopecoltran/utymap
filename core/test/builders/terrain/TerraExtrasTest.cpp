#include "builders/terrain/TerraExtras.hpp"
#include "entities/Area.hpp"

#include <boost/test/unit_test.hpp>
#include "test_utils/DependencyProvider.hpp"
#include "test_utils/ElementUtils.hpp"

#include <algorithm>

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::mapcss;
using namespace utymap::meshing;

namespace {
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
                std::bind(&Builders_Terrain_TerraExtrasFixture::verifyMesh, this, std::placeholders::_1),
                nullptr),
                isVerified(false)
        {
        }

        std::shared_ptr<Mesh> generateMesh()
        {
            auto mesh = std::make_shared<Mesh>("area");
            Polygon polygon(4, 0);
            polygon.addContour(std::vector < Vector2 > {{0, 0}, { 10, 0 }, { 10, 10 }, { 0, 10 } });
            MeshBuilder builder(*dependencyProvider.getElevationProvider());
            builder.addPolygon(*mesh, polygon, MeshBuilder::Options(5, 0, 0, 0, ColorGradient()));
            return mesh;
        }

        Style generateStyle()
        {
            Area area = ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(), 0, 
                { { "amenity", "forest" } },
                { { 0, 0 }, { 10, 0 }, { 10, 10 }, { 0, 10 } });
            return dependencyProvider.getStyleProvider(stylesheet)->forElement(area, 16);
        }

        void verifyMesh(const Mesh& mesh)
        {
            BOOST_CHECK_GT(mesh.vertices.size(), 0);
            BOOST_CHECK_GT(mesh.triangles.size(), 0);
            BOOST_CHECK_GT(mesh.colors.size(), 0);

            BOOST_CHECK_EQUAL(*std::max_element(mesh.triangles.begin(), mesh.triangles.end()), static_cast<int>(mesh.vertices.size() - 1) / 3);
            BOOST_CHECK_EQUAL(mesh.vertices.size() / 3, mesh.colors.size());

            isVerified = true;
        }

        DependencyProvider dependencyProvider;
        BuilderContext builderContext;
        bool isVerified;
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

    TerraExtras::addForest(builderContext, extrasContext);

    BOOST_CHECK(isVerified);
}

BOOST_AUTO_TEST_SUITE_END()