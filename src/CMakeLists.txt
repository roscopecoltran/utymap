set(HEADER_FILES
                mapcss/StyleSheet.hpp
                mapcss/Parser.hpp
                )

add_library (UtyMap
                mapcss/Parser.cpp
                ${HEADER_FILES}
                )

set_target_properties(UtyMap PROPERTIES LINKER_LANGUAGE CXX)
