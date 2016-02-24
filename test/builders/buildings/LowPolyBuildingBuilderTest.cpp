#include "builders/buildings/LowPolyBuildingBuilder.hpp"
#include "heightmap/ElevationProvider.hpp"
#include "index/StringTable.hpp"
#include "mapcss/StyleProvider.hpp"

#include "test_utils/MapCssUtils.hpp"

#include <boost/test/unit_test.hpp>
#include <cstdio>

using utymap::heightmap::ElevationProvider;
using utymap::index::StringTable;
using utymap::mapcss::StyleProvider;

class TestElevationProvider : public ElevationProvider
{
public:
    double getElevation(double x, double y) { return 0; }
};

struct Builders_Buildings_LowPolyBuildingsBuilderFixture
{
    Builders_Buildings_LowPolyBuildingsBuilderFixture() :
        stringTablePtr(new StringTable("")),
        styleProviderPtr(MapCssUtils::createStyleProviderFromString(*stringTablePtr, "")),
        eleProvider()
    {
    }

    ~Builders_Buildings_LowPolyBuildingsBuilderFixture()
    {
        delete styleProviderPtr;
        delete stringTablePtr;

        std::remove("string.idx");
        std::remove("string.dat");
    }

    TestElevationProvider eleProvider;
    StringTable* stringTablePtr;
    StyleProvider* styleProviderPtr;
};
