#include "config.hpp"
#include "ExportLib.h"

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

BOOST_AUTO_TEST_CASE(GivenTestData_WhenTileLoad_ThenCallbacksAreCalled)
{
    /*::loadTile(0, 0, 1,
        [](const char* name,
           const double* vertices, int vertexCount,
           const int* triangles, int triCount,
           const int* colors, int colorCount) {
            // TODO basic checks
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
    );*/
}

BOOST_AUTO_TEST_SUITE_END()
