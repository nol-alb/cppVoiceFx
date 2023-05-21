set(CurrLib AudioFileIO)

## set version number
set (${CurrLib}_VERSION_MAJOR "0" CACHE INTERNAL "")
set (${CurrLib}_VERSION_MINOR "0" CACHE INTERNAL "")
set (${CurrLib}_VERSION_PATCH "1" CACHE INTERNAL "")

# get wrapper source files
file(GLOB ${CurrLib}_SOURCES RELATIVE ${CMAKE_SOURCE_DIR}/src/${CurrLib} *.c;*.cpp;*.txt)
file(GLOB ${CurrLib}_HEADERS RELATIVE ${CMAKE_SOURCE_DIR}/src/${CurrLib} *.h;../incl/*.h)

# get library source files
add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/sndlib "${CMAKE_CURRENT_BINARY_DIR}/sndlib")
list (APPEND PROJECT_INCLUDES ${CMAKE_SOURCE_DIR}/3rdparty/sndlib)    

## add include directories
add_inc_directories ()

# build a static library
add_library(${CurrLib} STATIC ${${CurrLib}_SOURCES} ${${CurrLib}_HEADERS} ../inc/CConvbase.h)
set_target_properties(${CurrLib} PROPERTIES CLEAN_DIRECT_OUTPUT 1)

# set output name
set_target_properties(${CurrLib} PROPERTIES OUTPUT_NAME "${CurrLib}")
set_target_properties(${CurrLib} PROPERTIES PREFIX "lib")

target_link_libraries(${CurrLib} sndlib)
