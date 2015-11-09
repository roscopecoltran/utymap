set(HEADER_FILES
                mapcss/StyleSheet.hpp
                mapcss/Parser.hpp
                formats/pbf/fileformat.pb.h
                formats/pbf/osmformat.pb.h
                formats/pbf/OsmPbfReader.hpp
                formats/xml/OsmXmlParser.hpp
                )

add_library (UtyMap
                mapcss/Parser.cpp
                formats/pbf/fileformat.pb.cc
                formats/pbf/osmformat.pb.cc
                ${HEADER_FILES}
                )

set_target_properties(UtyMap PROPERTIES LINKER_LANGUAGE CXX)
