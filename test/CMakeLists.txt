find_package (Boost COMPONENTS unit_test_framework REQUIRED)

# Define test assets path
file (READ ${TEST_SOURCE}/config.hpp FILE_CONTENT)
string(REPLACE "_TEST_ASSETS_PATH_" "${TEST_SOURCE}/test_assets/"
        MODIFIED_FILE_CONTENT "${FILE_CONTENT}")
file(WRITE ${TEST_SOURCE}/config.hpp "${MODIFIED_FILE_CONTENT}")

include_directories (${MAIN_SOURCE}
                     ${LIB_SOURCE}
                     ${SHARED_SOURCE}
                     ${TEST_SOURCE}
                     ${Boost_INCLUDE_DIRS}
                     )

add_definitions (-DBOOST_TEST_DYN_LINK)

set (HEADER_FILES
                config.hpp
                formats/osm/TestOsmDataVisitor.hpp
                formats/shape/TestShapeDataVisitor.hpp
                )

add_executable (TestLauncher
                        main.cpp
                        BoundingBoxTest.cpp
                        ExportLibTest.cpp
                        builders/TerraBuilderTest.cpp
                        builders/LineGridSplitterTest.cpp
                        entities/ElementTest.cpp
                        formats/shape/ShapeParserTest.cpp
                        formats/shape/ShapeDataVisitorTest.cpp
                        formats/osm/MultipolygonProcessorTest.cpp
                        formats/osm/pbf/OsmPbfParserTest.cpp
                        formats/osm/xml/OsmXmlParserTest.cpp
                        index/ElementStoreTest.cpp
                        index/InMemoryElementStoreTest.cpp
                        index/StringTableTest.cpp
                        mapcss/MapCssParserTest.cpp
                        mapcss/StyleProviderTest.cpp
                        meshing/MeshBuilderTest.cpp
                        test_utils/ElementUtils.hpp
                        test_utils/MapCssUtils.hpp
                        utils/GeoUtilsTest.cpp
                        utils/GradientUtilsTest.cpp
                        utils/NoiseUtilsTest.cpp
                        ${HEADER_FILES}
                      )

target_link_libraries (TestLauncher
                        UtyMap
                        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                        ${PROTOBUF_LIBRARY}
                        ${ZLIB_LIBRARY}
                        )
