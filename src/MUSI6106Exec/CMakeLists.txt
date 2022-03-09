set (APP MUSI6106Exec)

set (AudioLIB AudioFileIO)
set (VibratoLIB Vibrato)

## get all source files (note: this is considered bad style, one could also add file names individually)
file(GLOB ${APP}_SOURCES RELATIVE ${CMAKE_SOURCE_DIR}/src/${APP} *.c;*.cpp;*.txt)
file(GLOB ${APP}_HEADERS RELATIVE ${CMAKE_SOURCE_DIR}/src/${APP} *.h;../inc/*.h)

## add include directories
add_inc_directories ()

## create the target
add_executable(${APP} ${${APP}_SOURCES} ${${APP}_HEADERS} ../VoicefxIn/CVoicefxIf.cpp ../inc/CVoicefxIf.h ../Tremolo/Tremolo.cpp ../inc/Tremolo.h)

# link the libraries
target_link_libraries(${APP} ${LIB} ${VibratoLIB} ${AudioLIB} ${SndLIB} ${OS_LIBS})

