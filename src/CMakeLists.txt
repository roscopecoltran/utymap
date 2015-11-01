set(HEADER_FILES
                mapcss/Stylesheet.hpp
                mapcss/Parser.hpp
                )

add_library (UtyMap
                ${HEADER_FILES}
                )

set_target_properties(UtyMap PROPERTIES LINKER_LANGUAGE CXX)
