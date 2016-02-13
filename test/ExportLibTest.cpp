#include "config.hpp"
#include "ExportLib.cpp"

#include <boost/test/unit_test.hpp>

#include <string>

struct ExportLibFixture {
    ExportLibFixture()
    {
        ::configure(TEST_ASSETS_PATH, TEST_MAPCSS_DEFAULT, TEST_ASSETS_PATH,
            [](const char* message) {
            BOOST_TEST_FAIL(message);
        });
        BOOST_TEST_MESSAGE("setup fixture");
        auto callback = [](const char* msg)
        {
            BOOST_CHECK(msg == nullptr);
        };

        ::addToInMemoryStore(TEST_SHAPE_NE_110M_LAND, 1, 1, callback);
        ::addToInMemoryStore(TEST_SHAPE_NE_110M_RIVERS, 1, 1, callback);
        ::addToInMemoryStore(TEST_SHAPE_NE_110M_LAKES, 1, 1, callback);
        //::addToInMemoryStore(TEST_SHAPE_NE_110M_ADMIN, 1, 1, callback);
        ::addToInMemoryStore(TEST_SHAPE_NE_110M_BORDERS, 1, 1, callback);
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

BOOST_AUTO_TEST_CASE(GivenTestData_WhenAllTilesAreLoaded_ThenCallbacksAreCalled)
{
    for (int i = 0; i <= 1; ++i) {
        for (int j = 0; j <= 1; j++) {
            ::loadTile(i, j, 1,
            [](const char* name,
               const double* vertices, int vertexCount,
               const int* triangles, int triCount,
               const int* colors, int colorCount) {
                 BOOST_CHECK_GT(vertexCount, 0);
                 BOOST_CHECK_GT(triCount, 0);
                 BOOST_CHECK_GT(colorCount, 0);
            },
            [](uint64_t id, const char* name,
               const char** tags, int size,
               const double* vertices, int vertexCount) {
               // TODO basic checks
            },
            [](const char* message)
            {
                 BOOST_TEST_FAIL(message);
            }
            );
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()
