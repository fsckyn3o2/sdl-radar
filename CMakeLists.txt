cmake_minimum_required(VERSION 3.31)
project(radar)

# Set C standard
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -lm -lfluidsynth")
set(CMAKE_C_FLAGS_DEBUG "-g -DDEBUG -O0")
set(CMAKE_C_FLAGS_RELEASE "-O2")

# Find SDL2 using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
pkg_check_modules(SDL2_GFX REQUIRED SDL2_gfx)
pkg_check_modules(GLEW REQUIRED glew)

# Find ALSA library
pkg_check_modules(ALSA REQUIRED alsa)
pkg_check_modules(fluidsynth REQUIRED)

# List all source files
set(SOURCE_FILES
        src/main.c
        src/main_constants.h
        src/radar.h
        src/radar.c
)

add_executable(radar ${SOURCE_FILES})

# Add include directory for header files
target_include_directories(radar PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_TTF_INCLUDE_DIRS}
        ${SDL2_GFX_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(radar
        ${SDL2_LIBRARIES}
        ${SDL2_TTF_LIBRARIES}
        ${SDL2_IMAGE_LIBRARIES}
        ${ALSA_LIBRARIES}
        ${SDL2_GFX_LIBRARIES}
)

# Include directories and compile options
target_compile_options(radar PRIVATE ${SDL2_CFLAGS_OTHER} ${SDL2_TTF_CFLAGS_OTHER} ${SDL2_IMAGE_CFLAGS_OTHER} ${ALSA_CFLAGS_OTHER})
target_link_directories(radar PRIVATE ${SDL2_LIBRARY_DIRS} ${SDL2_TTF_LIBRARY_DIRS} ${SDL2_IMAGE_LIBRARY_DIRS} ${ALSA_INCLUDE_DIRS})

# Set output directory
set_target_properties(radar PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Add run target
add_custom_target(run
        COMMAND ${CMAKE_BINARY_DIR}/bin/radar
        DEPENDS radar
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)