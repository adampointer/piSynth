cmake_minimum_required(VERSION 2.8)
project(piSynth)

set(piSynth_VERSION_MAJOR 0)
set(piSynth_VERSION_MINOR 4)
set(piSynth_DOCUMENT_ROOT ${CMAKE_INSTALL_PREFIX}/share/piSynth/scripts)

message("-- Checking architecture")
message("   ...${CMAKE_SYSTEM_PROCESSOR}")

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "armv6l")
    set(CMAKE_C_FLAGS "-Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s")
endif()

configure_file(
    "${piSynth_SOURCE_DIR}/src/config.h.in"
    "${piSynth_BINARY_DIR}/src/config.h"
    )
include_directories("${piSynth_BINARY_DIR}")
add_executable(piSynth src/main.c src/pcm.c src/midi.c src/mongoose.c
                       src/server.c src/filter.c src/lfo.c src/alsaconnect.c)

find_package(ALSA)
find_package(Threads)
target_link_libraries(piSynth m ${CMAKE_DL_LIBS} ${ALSA_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${piSynth_SOURCE_DIR}/Doxyfile.in ${piSynth_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc ALL
    ${DOXYGEN_EXECUTABLE} ${piSynth_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${piSynth_BINARY_DIR}
    COMMENT "Generating man pages" VERBATIM
    )
    install(DIRECTORY ${CMAKE_BINARY_DIR}/doc/man/ DESTINATION man)
endif(DOXYGEN_FOUND)

install(TARGETS piSynth DESTINATION bin)
install(DIRECTORY scripts DESTINATION share/piSynth)
