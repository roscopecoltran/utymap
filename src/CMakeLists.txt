set(HEADER_FILES
                BoundingBox.hpp
                GeoCoordinate.hpp
                QuadKey.hpp
                entities/Element.hpp
                entities/ElementVisitor.hpp
                entities/Node.hpp
                entities/Relation.hpp
                entities/Way.hpp
                entities/Area.hpp
                formats/OsmTypes.hpp
                formats/pbf/fileformat.pb.h
                formats/pbf/osmformat.pb.h
                formats/pbf/OsmPbfParser.hpp
                formats/xml/OsmXmlParser.hpp
                index/GeoStore.hpp
                index/GeoUtils.hpp
                index/StringTable.hpp
                index/StyleFilter.hpp
                mapcss/StyleSheet.hpp
                mapcss/MapCssParser.hpp
                utils/MurmurHash3.h
                )

add_library (UtyMap
                ${HEADER_FILES}
                formats/pbf/fileformat.pb.cc
                formats/pbf/osmformat.pb.cc
                index/GeoStore.cpp
                index/StringTable.cpp
                index/StyleFilter.cpp
                mapcss/MapCssParser.cpp
                utils/MurmurHash3.cpp
                )

set_target_properties(UtyMap PROPERTIES LINKER_LANGUAGE CXX)

include_directories (${SOURCE})
