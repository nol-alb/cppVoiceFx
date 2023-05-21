set (APP TestExec)

set (TestLIB gtest)

set (FilterLIB CombFilter)
set (DtwLIB Dtw)
set (VibratoLIB Vibrato)
set (AudioLIB AudioFileIO)
set (SndLIB sndlib)
set (FftLIB Fft)
set (FastConvLIB FastConv)


list (APPEND PROJECT_INCLUDES ${CMAKE_SOURCE_DIR}/3rdparty/googletest/${TestLIB}/include)
list (APPEND PROJECT_INCLUDES ${CMAKE_SOURCE_DIR}/3rdparty/googletest/${TestLIB})

##############################################################
## TestExec
file(GLOB ${APP}_SOURCES RELATIVE ${CMAKE_SOURCE_DIR}/src/Tests/${APP} Test.cpp)
file(GLOB ${APP}_HEADERS RELATIVE ${CMAKE_SOURCE_DIR}/src/Tests/${APP} *.h;../incl/*.h)
source_group (Main FILES ${APP}_SOURCES ${APP}_HEADERS)

# these are the individual tests
file(GLOB ${APP}_SOURCES_TEST RELATIVE ${CMAKE_SOURCE_DIR}/src/Tests/${APP} ../Tests/*.cpp;)
file(GLOB ${APP}_HEADERS_TEST RELATIVE ${CMAKE_SOURCE_DIR}/src/Tests/${APP} ../Tests/*.h)
source_group (Tests FILES ${APP}_SOURCES_TEST ${APP}_HEADERS_TEST)

## add include directories
add_inc_directories ()

add_executable(${APP} ${${APP}_SOURCES} ${${APP}_HEADERS} ${${APP}_SOURCES_TEST} ${${APP}_HEADERS_TEST})

# link the libraries
target_link_libraries(${APP} ${LIB} ${TestLIB} ${FftLIB} ${FastConvLIB} ${VibratoLIB} ${FilterLIB} ${AudioLIB} ${SndLIB} ${OS_LIBS})


