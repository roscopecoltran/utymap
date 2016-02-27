#include "config.hpp"
#include "ExportLib.cpp"

#include <boost/test/unit_test.hpp>

#include <string>

struct ExportLibFixture {
    ExportLibFixture()
    {
        ::configure(TEST_ASSETS_PATH, TEST_ASSETS_PATH, [](const char* message) {
            BOOST_TEST_FAIL(message);
        });
        ::registerElementBuilder("place");
    }

    void loadTiles(int levelOfDetails, int startX, int endX, int startY, int endY)
    {
        for (int i = startX; i <= endX; ++i) {
            for (int j = startY; j <= endY; j++) {
                ::loadTile(TEST_MAPCSS_DEFAULT, i, j, levelOfDetails,
                    [](const char* name, const double* vertices, int vertexCount,
                       const int* triangles, int triCount, const int* colors, int colorCount) {
                    BOOST_CHECK_GT(vertexCount, 0);
                    BOOST_CHECK_GT(triCount, 0);
                    BOOST_CHECK_GT(colorCount, 0);
                },
                    [](uint64_t id, const char** tags, int size, const double* vertices,
                       int vertexCount, const char** style, int styleSize) {
                    // TODO basic checks
                },
                    [](const char* message) {
                    BOOST_TEST_FAIL(message);
                }
                );
            }
        }
    }

    ~ExportLibFixture()
    {
        BOOST_TEST_MESSAGE("teardown fixture");
        ::cleanup();
        std::remove((std::string(TEST_ASSETS_PATH) + "string.idx").c_str());
        std::remove((std::string(TEST_ASSETS_PATH) + "string.dat").c_str());
    }
};

BOOST_FIXTURE_TEST_SUITE(ExportLib, ExportLibFixture)

BOOST_AUTO_TEST_CASE(GivenTestData_WhenAllTilesAreLoadingAtZoomOne_ThenCallbacksAreCalled)
{
    auto callback = [](const char* msg) { BOOST_CHECK(msg == nullptr); };
    ::addToInMemoryStore(TEST_MAPCSS_DEFAULT, TEST_SHAPE_NE_110M_LAND, 1, 1, callback);
    ::addToInMemoryStore(TEST_MAPCSS_DEFAULT, TEST_SHAPE_NE_110M_RIVERS, 1, 1, callback);
    ::addToInMemoryStore(TEST_MAPCSS_DEFAULT, TEST_SHAPE_NE_110M_LAKES, 1, 1, callback);
    //::addToInMemoryStore(TEST_MAPCSS_DEFAULT, TEST_SHAPE_NE_110M_ADMIN, 1, 1, callback);
    ::addToInMemoryStore(TEST_MAPCSS_DEFAULT, TEST_SHAPE_NE_110M_BORDERS, 1, 1, callback);
    ::addToInMemoryStore(TEST_MAPCSS_DEFAULT, TEST_SHAPE_NE_110M_POPULATED_PLACES, 1, 1, callback);

    loadTiles(1, 0, 1, 0, 1);
}

BOOST_AUTO_TEST_CASE(GivenTestData_WhenAllTilesAreLoadingAtZoomNineteen_ThenCallbacksAreCalled)
{
    auto callback = [](const char* msg) { BOOST_CHECK(msg == nullptr); };
    ::addToInMemoryStore(TEST_MAPCSS_DEFAULT, TEST_XML_FILE, 19, 19, callback);

    loadTiles(19, 281640, 281640, 171904, 171904); 
}

BOOST_AUTO_TEST_SUITE_END()
