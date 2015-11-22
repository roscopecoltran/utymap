find_package (Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

# Define test assets path
file (READ ${TEST_SOURCE}/config.hpp FILE_CONTENT)
string(REPLACE "_TEST_ASSETS_PATH_" "${TEST_SOURCE}/test_assets/"
        MODIFIED_FILE_CONTENT "${FILE_CONTENT}")
file(WRITE ${TEST_SOURCE}/config.hpp "${MODIFIED_FILE_CONTENT}")

include_directories (${SOURCE}
                     ${TEST_SOURCE}
                     ${Boost_INCLUDE_DIRS}
                     )

add_definitions (-DBOOST_TEST_DYN_LINK)

set (HEADER_FILES
                config.hpp
                formats/OsmDataVisitor.hpp
                )

add_executable (TestLauncher
                        main.cpp
                        BoundingBoxTest.cpp
                        entities/ElementTest.cpp
                        formats/pbf/OsmPbfParserTest.cpp
                        formats/xml/OsmXmlParserTest.cpp
                        index/GeoUtilsTest.cpp
                        index/StringTableTest.cpp
                        mapcss/ParserTest.cpp
                        ${HEADER_FILES}
                      )

target_link_libraries (TestLauncher
                        UtyMap
                        ${Boost_FILESYSTEM_LIBRARY}
                        ${Boost_SYSTEM_LIBRARY}
                        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                        ${PROTOBUF_LIBRARY}
                        ${ZLIB_LIBRARY}
                        )
