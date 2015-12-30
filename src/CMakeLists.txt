set(HEADER_FILES
                BoundingBox.hpp
                Exceptions.hpp
                ExportLib.h
                GeoCoordinate.hpp
                QuadKey.hpp
                TileLoader.hpp
                entities/Element.hpp
                entities/ElementVisitor.hpp
                entities/Node.hpp
                entities/Relation.hpp
                entities/Way.hpp
                entities/Area.hpp
                formats/FormatTypes.hpp
                formats/pbf/fileformat.pb.h
                formats/pbf/osmformat.pb.h
                formats/pbf/OsmPbfParser.hpp
                formats/shape/shapefil.h
                formats/shape/ShapeParser.hpp
                formats/xml/OsmXmlParser.hpp
                heightmap/ElevationProvider.hpp
                heightmap/FlatElevationProvider.hpp
                index/GeoStore.hpp
                index/GeoUtils.hpp
                index/StringTable.hpp
                index/StyleFilter.hpp
                mapcss/StyleSheet.hpp
                mapcss/MapCssParser.hpp
                meshing/clipper.hpp
                meshing/MeshBuilder.hpp
                meshing/MeshTypes.hpp
                meshing/Polygon.hpp
                meshing/Triangle.h
                terrain/LineGridSplitter.hpp
                terrain/MeshRegion.hpp
                terrain/TerraBuilder.hpp
                utils/CoreUtils.hpp
                utils/MurmurHash3.h
                )

add_library (UtyMap
                ${HEADER_FILES}
                TileLoader.cpp
                formats/pbf/fileformat.pb.cc
                formats/pbf/osmformat.pb.cc
                formats/shape/dbfopen.c
                formats/shape/safileio.c
                formats/shape/shpopen.c
                index/GeoStore.cpp
                index/StringTable.cpp
                index/StyleFilter.cpp
                mapcss/MapCssParser.cpp
                mapcss/StyleSheet.cpp
                meshing/clipper.cpp
                meshing/Triangle.c
                meshing/MeshBuilder.cpp
                terrain/TerraBuilder.cpp
                utils/MurmurHash3.cpp
                )

set_target_properties(UtyMap PROPERTIES LINKER_LANGUAGE CXX)

include_directories (${SOURCE})
