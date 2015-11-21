set(HEADER_FILES
                CommonTypes.hpp
                formats/OsmTypes.hpp
                formats/pbf/fileformat.pb.h
                formats/pbf/osmformat.pb.h
                formats/pbf/OsmPbfParser.hpp
                formats/xml/OsmXmlParser.hpp
                index/GeoUtils.hpp
                index/StringTable.hpp
                mapcss/StyleSheet.hpp
                mapcss/Parser.hpp
                utils/MurmurHash3.h
                )

add_library (UtyMap
                ${HEADER_FILES}
                formats/pbf/fileformat.pb.cc
                formats/pbf/osmformat.pb.cc
                index/StringTable.cpp
                mapcss/Parser.cpp
                utils/MurmurHash3.cpp
                )

set_target_properties(UtyMap PROPERTIES LINKER_LANGUAGE CXX)
