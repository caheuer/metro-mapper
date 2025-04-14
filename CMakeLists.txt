cmake_minimum_required(VERSION 3.10)
project(metro_mapper)

set(CMAKE_CXX_STANDARD 17)

if(EMSCRIPTEN)
    message(STATUS "Configuring for Emscripten (Web)")

    set(CMAKE_EXECUTABLE_SUFFIX ".html")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
        -sUSE_SDL=2 \
        -sUSE_SDL_IMAGE=2 \
        -sUSE_SDL_TTF=2 \
        -sUSE_SDL_GFX=2 \
        -sSDL2_IMAGE_FORMATS=[\"png\"]")

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
        -sASYNCIFY \
        -sFORCE_FILESYSTEM=1 \
        --preload-file assets")

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
        -sEXPORTED_FUNCTIONS=_main,_onFileLoaded,_onFileError,_allocFileBuffer \
        -sEXPORTED_RUNTIME_METHODS=ccall,cwrap")
else()
    message(STATUS "Configuring for native build")

    # Use pkg-config to find SDL2 and SDL2_gfx
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)
    pkg_check_modules(SDL2_GFX REQUIRED SDL2_gfx)
    pkg_check_modules(SDL2_ttf REQUIRED SDL2_ttf)
    pkg_check_modules(SDL2_image REQUIRED SDL2_image)

    # Include directories
    include_directories(${SDL2_INCLUDE_DIRS})
    include_directories(${SDL2_GFX_INCLUDE_DIRS})
    include_directories(${SDL2_ttf_INCLUDE_DIRS})
    include_directories(${SDL2_image_INCLUDE_DIRS})

    # Library directories
    link_directories(${SDL2_LIBRARY_DIRS})
    link_directories(${SDL2_GFX_LIBRARY_DIRS})
    link_directories(${SDL2_ttf_LIBRARY_DIRS})
    link_directories(${SDL2_image_LIBRARY_DIRS})
endif()

# Automatically include all source/header files from src/
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.hpp")

# External libraries
include_directories(${CMAKE_SOURCE_DIR}/lib)
list(APPEND SOURCES "${CMAKE_SOURCE_DIR}/lib/tinyfiledialogs.c")

# Define executable
add_executable(metro_mapper ${SOURCES})

# Link libraries
if(NOT EMSCRIPTEN)
    target_link_libraries(metro_mapper
        ${SDL2_LIBRARIES}
        ${SDL2_GFX_LIBRARIES}
        ${SDL2_ttf_LIBRARIES}
        ${SDL2_image_LIBRARIES}
    )
endif()
