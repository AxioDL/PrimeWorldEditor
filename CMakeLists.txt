cmake_minimum_required(VERSION 3.12)
set(MACOSX_DEPLOYMENT_TARGET 10.10)

# Use release build type as default (assimp complains if this is unset)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

project(PrimeWorldEditor CXX)

include(./dew.cmake)
integrate_dew()

include(cmake/generate_product_version.cmake)

if(MSVC)
    add_compile_options(/WX /wd4267 /wd4100 /wd4101 /wd4189)
else()
    add_compile_options(-Wno-multichar)
endif()

if(APPLE)
    add_compile_definitions(GL_SILENCE_DEPRECATION)
endif()

# Set where the binary files will be built.  The program will not execute from
# here.  You must run "make install" to install these to the proper location
# as defined above.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src/Core)
add_subdirectory(src/Editor)
