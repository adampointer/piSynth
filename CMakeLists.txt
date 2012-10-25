cmake_minimum_required(VERSION 2.8)
project(piSynth)
set(piSynth_VERSION_MAJOR 0)
set(piSynth_VERSION_MINOR 3)
set(piSynth_DOCUMENT_ROOT ${CMAKE_INSTALL_PREFIX}/share/piSynth/scripts)
configure_file(
    "${piSynth_SOURCE_DIR}/src/config.h.in"
    "${piSynth_BINARY_DIR}/src/config.h"
    )
include_directories("${piSynth_BINARY_DIR}")
add_executable(piSynth src/main.c src/pcm.c src/midi.c src/mongoose.c src/server.c)
target_link_libraries(piSynth m asound pthread)
install(TARGETS piSynth DESTINATION bin)
install(DIRECTORY scripts DESTINATION share/piSynth)
