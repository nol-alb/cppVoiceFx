set (APP TestExec)

set (TestLIB gtest)

set (VibratoLIB Vibrato)
set (AudioLIB AudioFileIO)
set (SndLIB sndlib)


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

# if (WITH_TESTS)
    add_executable(${APP} ${${APP}_SOURCES} ${${APP}_HEADERS} ${${APP}_SOURCES_TEST} ${${APP}_HEADERS_TEST} ../../VoicefxIn/CVoicefxIf.cpp ../../inc/CVoicefxIf.h ../../Tremolo/Tremolo.cpp ../../inc/Tremolo.h)

    # link the libraries
    target_link_libraries(${APP} ${LIB} ${TestLIB} ${VibratoLIB} ${AudioLIB} ${SndLIB} ${OS_LIBS})
#endif (WITH_TESTS)


