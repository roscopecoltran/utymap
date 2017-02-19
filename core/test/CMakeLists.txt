
# added by Luc Michalski - 2017

# ---[ Boost
IF(HUNTER_ENABLED)
    hunter_add_package(Boost COMPONENTS test system filesystem)
    find_package(Boost CONFIG REQUIRED COMPONENTS unit_test_framework system filesystem)
ELSE()
    find_package(Boost COMPONENTS unit_test_framework system filesystem REQUIRED)
ENDIF(HUNTER_ENABLED)

enable_testing ()

# Define test assets path
file(READ ${TEST_SOURCE}/config.hpp FILE_CONTENT)

string( REPLACE "_TEST_ASSETS_PATH_" "${TEST_SOURCE}/test_assets/"
        MODIFIED_FILE_CONTENT "${FILE_CONTENT}")

file(WRITE ${TEST_SOURCE}/config.hpp "${MODIFIED_FILE_CONTENT}")

# ---[ Protobuf
IF(HUNTER_ENABLED)

    hunter_add_package(Protobuf)
    find_package(Protobuf CONFIG REQUIRED)  

    set(PROTOS_TEST_FILES "${MAIN_SOURCE}/formats/osm/pbf/fileformat.proto" "${MAIN_SOURCE}/formats/osm/pbf/osmformat.proto")
    set(PROTO_TEST_SRCS)
    set(PROTO_TEST_HDRS)

    #Code Generation
    foreach(proto_test_file ${PROTOS_TEST_FILES})
      get_filename_component(proto_test_file_abs ${proto_test_file} ABSOLUTE)
      get_filename_component(basename ${proto_test_file} NAME_WE)
      set(generated_files ${basename}.pb.cc ${basename}.pb.h)

      list(APPEND PROTO_TEST_SRCS ${basename}.pb.cc)
      list(APPEND PROTO_TEST_HDRS ${basename}.pb.h)

      add_custom_command(
          OUTPUT ${generated_files}
          COMMAND protobuf::protoc --cpp_out ${CMAKE_CURRENT_BINARY_DIR} -I ${CMAKE_CURRENT_SOURCE_DIR} ${proto_test_file_abs}
          COMMENT "Generating ${generated_files} from ${proto_test_file}"
          VERBATIM
      )

    endforeach()
    add_library(protos_test ${PROTO_TEST_SRCS})
    target_link_libraries(protos_test protobuf::libprotobuf)
    target_include_directories(protos_test PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/..)

ELSE()

    PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS
        ${MAIN_SOURCE}/formats/osm/pbf/fileformat.proto
        ${MAIN_SOURCE}/formats/osm/pbf/osmformat.proto)

ENDIF(HUNTER_ENABLED)

include_directories(${MAIN_SOURCE}
        ${LIB_SOURCE}
        ${SHARED_SOURCE}
        ${TEST_SOURCE}
        ${Boost_INCLUDE_DIRS}
        ${CMAKE_CURRENT_BINARY_DIR}
        )

add_definitions(-DBOOST_TEST_DYN_LINK)

set(HEADER_FILES
        ${PROTO_HDRS}
        config.hpp
        formats/osm/CountableOsmDataVisitor.hpp
        formats/shape/CountableShapeDataVisitor.hpp
        lsys/StringTurtle.hpp
        test_utils/DependencyProvider.hpp
        test_utils/ElementUtils.hpp
        )

set (TEST Test)

add_executable(${TEST}
        ${PROTO_SRCS}
        main.cpp
        BoundingBoxTest.cpp
        ExportLibTest.cpp
        builders/buildings/BuildingBuilderTest.cpp
        builders/buildings/RoofBuildersTest.cpp
        builders/generators/GeneratorTest.cpp
        builders/poi/TreeBuilderTest.cpp
        builders/misc/BarrierBuilderTest.cpp
        builders/terrain/LineGridSplitterTest.cpp
        builders/terrain/TerraBuilderTest.cpp
        builders/terrain/TerraExtrasTest.cpp
        entities/ElementTest.cpp
        formats/shape/ShapeParserTest.cpp
        formats/shape/ShapeDataVisitorTest.cpp
        formats/osm/MultipolygonProcessorTest.cpp
        formats/osm/OsmDataVisitorTest.cpp
        formats/osm/json/OsmJsonParserTest.cpp
        formats/osm/pbf/OsmPbfParserTest.cpp
        formats/osm/xml/OsmXmlParserTest.cpp
        heightmap/GridElevationProviderTest.cpp
        heightmap/SrtmElevationProviderTest.cpp
        index/ElementStoreTest.cpp
        index/InMemoryElementStoreTest.cpp
        index/PersistentElementStoreTest.cpp
        index/StringTableTest.cpp
        lsys/LSystemParserTest.cpp
        lsys/RulesTest.cpp
        lsys/TurtleTest.cpp
        mapcss/MapCssParserTest.cpp
        mapcss/StyleDeclarationTest.cpp
        mapcss/StyleProviderTest.cpp
        mapcss/StyleTest.cpp
        meshing/MeshBuilderTest.cpp
        utils/GeometryUtilsTest.cpp
        utils/GeoUtilsTest.cpp
        utils/GradientUtilsTest.cpp
        utils/NoiseUtilsTest.cpp
        ${HEADER_FILES}
        )

IF(HUNTER_ENABLED)
    target_link_libraries(${TEST} UtyMap Boost::unit_test_framework Boost::system Boost::filesystem protos_test)
ELSE()
    target_link_libraries(${TEST} UtyMap
                                  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                                  ${Boost_SYSTEM_LIBRARY}
                                  ${Boost_FILESYSTEM_LIBRARY})    
ENDIF()

enable_testing ()
add_test (${TEST} ${TEST})
