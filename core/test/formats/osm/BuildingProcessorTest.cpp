#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "formats/osm/BuildingProcessor.hpp"
#include "test_utils/ElementUtils.hpp"

#include <boost/test/unit_test.hpp>
#include "test_utils/DependencyProvider.hpp"

struct Formats_Osm_BuildingProcessorFixture
{
    Formats_Osm_BuildingProcessorFixture() :
            dependencyProvider()
    {
    }

    DependencyProvider dependencyProvider;
};

BOOST_FIXTURE_TEST_SUITE(Formats_Osm_BuildingProcessor, Formats_Osm_BuildingProcessorFixture)

BOOST_AUTO_TEST_CASE(Test_Stub)
{
    // TODO
}

BOOST_AUTO_TEST_SUITE_END()
