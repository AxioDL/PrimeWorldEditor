cmake_minimum_required(VERSION 3.12)

project(pwe_core CXX C)

find_package(tinyxml2 CONFIG REQUIRED)
find_package(logvisor CONFIG REQUIRED)
find_package(nod CONFIG REQUIRED)
find_package(lzokay CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)

file(GLOB_RECURSE source_files
    "*.c"
    "*.cpp"
    "*.h"
    "*.hpp"
)

add_library(pwe_core ${source_files})

target_compile_features(pwe_core PRIVATE cxx_std_17)

target_include_directories(pwe_core
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/..>
        ${CMAKE_CURRENT_SOURCE_DIR}
        OpenGL
)

target_link_libraries(
    pwe_core
    libcommon
    nod::nod
    logvisor::logvisor
    lzokay::lzokay
    OpenGL::GL
    assimp::assimp
    Threads::Threads
    ${ZLIB_LIBRARY}
)

target_compile_definitions(
    pwe_core
    PRIVATE
        USE_LZOKAY
    PUBLIC
        UNICODE
        GLEW_STATIC
)

gather_include_directories(pwe_core_include_directories pwe_core)

add_codegen_targets(
    "${source_files}"
    codegen_generated_files
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_BINARY_DIR}"
    "${pwe_core_include_directories}"
)
add_custom_target(pwe_core_codegen DEPENDS ${codegen_generated_files} SOURCES ${source_files})

# Add the generated sources to the library target
target_sources(pwe_core PRIVATE ${codegen_generated_files})
