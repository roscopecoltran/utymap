include_directories(${MAIN_SOURCE})

set(LIBRARY_NAME UtyMapLib)

add_library(${LIBRARY_NAME} SHARED Application.hpp ExportLib.cpp)

set_target_properties(${LIBRARY_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(${LIBRARY_NAME} UtyMap)
