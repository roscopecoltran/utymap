include_directories(${LIBRARY_SOURCE})

add_library(UtyMapLib SHARED ExportLib.cpp)
set_target_properties(UtyMapLib PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(UtyMapLib UtyMap)
