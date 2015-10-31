set(HEADER_FILES
                mapcss/Stylesheet.hpp
                mapcss/Parser.hpp
                )

add_library (UtyMap
                mapcss/Stylesheet.cpp
                mapcss/Parser.cpp
                ${HEADER_FILES}
                )
