set (APP MUSI6106Exec)

set (AudioLIB AudioFileIO)
set (CombFilterLIB CombFilter)
set (FftLIB Fft)
set (VibratoLIB Vibrato)
set (FastConvLIB FastConv)

## get all source files (note: this is considered bad style, one could also add file names individually)
file(GLOB ${APP}_SOURCES RELATIVE ${CMAKE_SOURCE_DIR}/src/${APP} *.c;*.cpp;*.txt)
file(GLOB ${APP}_HEADERS RELATIVE ${CMAKE_SOURCE_DIR}/src/${APP} *.h;../inc/*.h)

## add include directories
add_inc_directories ()

## create the target
add_executable(${APP} ${${APP}_SOURCES} ${${APP}_HEADERS})

# link the libraries
target_link_libraries(${APP} ${LIB} ${AudioLIB} ${FftLIB} ${FastConvLIB} ${VibratoLIB} ${CombFilterLIB} ${SndLIB} ${OS_LIBS})

