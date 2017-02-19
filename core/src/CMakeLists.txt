
set(LIBRARY_NAME UtyMap)

if(ENABLE_UNITYPLUGIN)

    # use the project name if the target as
    if(NOT DEFINED HUNTER_UNITY3D_PLUGIN_SDK_NAME)
        set(HUNTER_UNITY3D_PLUGIN_SDK_NAME "${LIBRARY_NAME}ForUnity")
    endif(NOT DEFINED HUNTER_UNITY3D_PLUGIN_SDK_NAME)    

    if(NOT DEFINED HUNTER_UNITY3D_PLUGIN_LIBRARY_NAME)
        set(HUNTER_UNITY3D_PLUGIN_LIBRARY_NAME ${LIBRARY_NAME})
    endif(NOT DEFINED HUNTER_UNITY3D_PLUGIN_LIBRARY_NAME)

    list(APPEND CMAKE_MODULE_PATH "${current_project_root_path}/cmake/Unity3D") 
    include(Unity3DNativePluginExport) # will set suffix and prefix for each platforms

    hunter_unity3d_set_target_properties(
        SDK_NAME          "${HUNTER_UNITY3D_PLUGIN_SDK_NAME}"
        LIBRARY_NAME      "${HUNTER_UNITY3D_PLUGIN_LIBRARY_NAME}"
        #ARCH_PREFIX_ALIAS "x86" # eg i386 -> x86
    )

endif(ENABLE_UNITYPLUGIN)

IF(HUNTER_ENABLED)

    hunter_add_package(Protobuf)
    find_package(Protobuf CONFIG REQUIRED)  

    set(PROTOS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/formats/osm/pbf/fileformat.proto ${CMAKE_CURRENT_SOURCE_DIR}/formats/osm/pbf/osmformat.proto)
    set(PROTO_SRCS)
    set(PROTO_HDRS)

    #Code Generation
    foreach(proto_file ${PROTOS_FILES})
      get_filename_component(proto_file_abs ${proto_file} ABSOLUTE)
      MESSAGE(STATUS " ** [Sniper] Protobuf generation - proto_file_abs: ${proto_file_abs}")
      get_filename_component(basename ${proto_file} NAME_WE)
      MESSAGE(STATUS " ** [Sniper] Protobuf generation - basename: ${basename}")
      set(generated_files ${basename}.pb.cc ${basename}.pb.h)
      MESSAGE(STATUS " ** [Sniper] Protobuf generation - generated_files: ${generated_files}")

      list(APPEND PROTO_SRCS ${basename}.pb.cc)
      list(APPEND PROTO_HDRS ${CMAKE_CURRENT_BINARY_DIR}/${basename}.pb.h)

      MESSAGE(STATUS " ** [Sniper] Protobuf generation - generated_files: ${generated_files}")
      MESSAGE(STATUS " ** [Sniper] Protobuf generation - command: protobuf::protoc --cpp_out ${CMAKE_CURRENT_BINARY_DIR} -I ${CMAKE_CURRENT_SOURCE_DIR} ${proto_file_abs}")
      MESSAGE(STATUS " ** [Sniper] Protobuf generation - Generating ${generated_files} from ${proto_file}")

      add_custom_command(
          OUTPUT ${generated_files}
          COMMAND protobuf::protoc --cpp_out ${CMAKE_CURRENT_BINARY_DIR} -I ${CMAKE_CURRENT_SOURCE_DIR}/formats/osm/pbf/ ${proto_file_abs}
          COMMENT "Generating ${generated_files} from ${proto_file}"
          VERBATIM
      )
    endforeach()

    MESSAGE(STATUS " ** [Sniper] Protobuf generation - protos: ${PROTO_SRCS}")
    add_library(protos ${PROTO_SRCS})
    target_link_libraries(protos protobuf::libprotobuf)

    #target_include_directories(protos PUBLIC ".")
    #add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/formats/osm/pbf)

ELSE()
    PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${MAIN_SOURCE}/formats/osm/pbf/fileformat.proto ${MAIN_SOURCE}/formats/osm/pbf/osmformat.proto)
ENDIF(HUNTER_ENABLED)

set(HEADER_FILES
        ${PROTO_HDRS}
        ${LIB_SOURCE}/clipper/clipper.hpp
        ${LIB_SOURCE}/hashing/MurmurHash3.h
        ${LIB_SOURCE}/triangle/triangle.h
        ${LIB_SOURCE}/shapefile/shapefil.h
        BoundingBox.hpp
        Exceptions.hpp
        GeoCoordinate.hpp
        LodRange.hpp
        QuadKey.hpp
        builders/BuilderContext.hpp
        builders/ElementBuilder.hpp
        builders/ExternalBuilder.hpp
        builders/MeshBuilder.hpp
        builders/MeshContext.hpp
        builders/QuadKeyBuilder.hpp
        builders/buildings/BuildingBuilder.hpp
        builders/buildings/facades/CylinderFacadeBuilder.hpp
        builders/buildings/facades/FacadeBuilder.hpp
        builders/buildings/facades/FlatFacadeBuilder.hpp
        builders/buildings/facades/SphereFacadeBuilder.hpp
        builders/buildings/roofs/DomeRoofBuilder.hpp
        builders/buildings/roofs/FlatRoofBuilder.hpp
        builders/buildings/roofs/MansardRoofBuilder.hpp
        builders/buildings/roofs/PyramidalRoofBuilder.hpp
        builders/buildings/roofs/RoofBuilder.hpp
        builders/buildings/roofs/RoundRoofBuilder.hpp
        builders/buildings/roofs/SkillionRoofBuilder.hpp
        builders/generators/AbstractGenerator.hpp
        builders/generators/CylinderGenerator.hpp
        builders/generators/IcoSphereGenerator.hpp
        builders/generators/LSystemGenerator.hpp
        builders/generators/WallGenerator.hpp
        builders/misc/BarrierBuilder.hpp
        builders/misc/LampBuilder.hpp
        builders/misc/LaneBuilder.hpp
        builders/poi/TreeBuilder.hpp
        builders/terrain/ExteriorGenerator.hpp
        builders/terrain/LineGridSplitter.hpp
        builders/terrain/RegionTypes.hpp
        builders/terrain/SurfaceGenerator.hpp
        builders/terrain/TerraBuilder.hpp
        builders/terrain/TerraExtras.hpp
        builders/terrain/TerraGenerator.hpp
        entities/Element.hpp
        entities/ElementVisitor.hpp
        entities/Node.hpp
        entities/Relation.hpp
        entities/Way.hpp
        entities/Area.hpp
        formats/FormatTypes.hpp
        formats/osm/BuildingProcessor.hpp
        formats/osm/MultipolygonProcessor.hpp
        formats/osm/OsmDataContext.hpp
        formats/osm/OsmDataVisitor.hpp
        formats/osm/RelationProcessor.hpp
        formats/osm/json/OsmJsonParser.hpp
        formats/osm/pbf/OsmPbfParser.hpp
        formats/osm/xml/OsmXmlParser.hpp
        formats/shape/ShapeParser.hpp
        formats/shape/ShapeDataVisitor.hpp
        heightmap/ElevationProvider.hpp
        heightmap/FlatElevationProvider.hpp
        heightmap/GridElevationProvider.hpp
        heightmap/SrtmElevationProvider.hpp
        index/ElementGeometryClipper.hpp
        index/ElementStore.hpp
        index/GeoStore.hpp
        index/InMemoryElementStore.hpp
        index/PersistentElementStore.hpp
        index/StringTable.hpp
        lsys/Turtle3d.hpp
        lsys/LSystem.hpp
        lsys/LSystemParser.hpp
        lsys/Rules.hpp
        lsys/Turtle.hpp
        mapcss/Color.hpp
        mapcss/ColorGradient.hpp
        mapcss/MapCssParser.hpp
        mapcss/StyleSheet.hpp
        mapcss/Style.hpp
        mapcss/StyleConsts.hpp
        mapcss/StyleEvaluator.hpp
        mapcss/StyleDeclaration.hpp
        mapcss/StyleProvider.hpp
        mapcss/TextureAtlasParser.hpp
        math/LineLinear.hpp
        math/Mesh.hpp
        math/Polygon.hpp
        math/Quaternion.hpp
        math/Rectangle.hpp
        math/Vector2.hpp
        math/Vector3.hpp
        utils/CoreUtils.hpp
        utils/ElementUtils.hpp
        utils/GeometryUtils.hpp
        utils/GeoUtils.hpp
        utils/GradientUtils.hpp
        utils/MathUtils.hpp
        utils/MeshUtils.hpp
        utils/NoiseUtils.hpp
        utils/SvgBuilder.hpp
        )

add_library(${LIBRARY_NAME}
        ${HEADER_FILES}
        ${PROTO_SRCS}
        ${LIB_SOURCE}/clipper/clipper.cpp
        ${LIB_SOURCE}/hashing/MurmurHash3.cpp
        ${LIB_SOURCE}/triangle/triangle.c
        ${LIB_SOURCE}/shapefile/dbfopen.c
        ${LIB_SOURCE}/shapefile/safileio.c
        ${LIB_SOURCE}/shapefile/shpopen.c
        builders/MeshBuilder.cpp
        builders/generators/IcoSphereGenerator.cpp
        builders/generators/LSystemGenerator.cpp
        builders/misc/BarrierBuilder.cpp
        builders/misc/LampBuilder.cpp
        builders/misc/LaneBuilder.cpp
        builders/poi/TreeBuilder.cpp
        builders/terrain/SurfaceGenerator.cpp
        builders/terrain/ExteriorGenerator.cpp
        builders/terrain/TerraBuilder.cpp
        builders/terrain/TerraExtras.cpp
        builders/terrain/TerraGenerator.cpp
        builders/QuadKeyBuilder.cpp
        builders/buildings/BuildingBuilder.cpp
        formats/osm/MultipolygonProcessor.cpp
        formats/osm/OsmDataVisitor.cpp
        index/ElementGeometryClipper.cpp
        index/ElementStore.cpp
        index/GeoStore.cpp
        index/InMemoryElementStore.cpp
        index/PersistentElementStore.cpp
        index/StringTable.cpp
        lsys/Turtle3d.cpp
        lsys/LSystemParser.cpp
        lsys/Turtle.cpp
        mapcss/MapCssParser.cpp
        mapcss/StyleConsts.cpp
        mapcss/StyleEvaluator.cpp
        mapcss/StyleProvider.cpp
        mapcss/TextureAtlasParser.cpp
        utils/GradientUtils.cpp
        utils/NoiseUtils.cpp
        )

set_target_properties(${LIBRARY_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(${LIBRARY_NAME} PROPERTIES LINKER_LANGUAGE CXX)

IF(HUNTER_ENABLED)
    target_link_libraries(${LIBRARY_NAME} protos ZLIB::zlib)
ELSE()
    target_link_libraries(${LIBRARY_NAME} ${PROTOBUF_LIBRARY} ${ZLIB_LIBRARY})
ENDIF()

include_directories(${MAIN_SOURCE} ${LIB_SOURCE} ${CMAKE_CURRENT_BINARY_DIR})

# ---[ Install main target 


# ---[ Deploy Unity3D plugin

    hunter_unity3d_deploy_target(
        SDK_NAME          "${HUNTER_UNITY3D_PLUGIN_SDK_NAME}"
        LIBRARY_NAME      "${HUNTER_UNITY3D_PLUGIN_LIBRARY_NAME}"
        #ARCH_PREFIX_ALIAS "x86" # eg i386 -> x86
    )



