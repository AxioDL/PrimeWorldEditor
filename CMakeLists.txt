cmake_minimum_required(VERSION 3.12)
set(MACOSX_DEPLOYMENT_TARGET 10.10)

# Use release build type as default (assimp complains if this is unset)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE})

project(PrimeWorldEditor CXX)

option(PWE_PUBLIC_RELEASE "Enable end-user deployment configuration for PWE" ON)
if (PWE_PUBLIC_RELEASE)
    add_compile_definitions(PUBLIC_RELEASE=1)
    message(STATUS "Enabled public release mode")
endif()

# Ensure submodules checked out
if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/externals/LibCommon/CMakeLists.txt)
    message(FATAL_ERROR "Please run 'git submodule update --init --recursive' to fetch submodules.")
endif()

include(./dew.cmake)
integrate_dew()

include(cmake/generate_product_version.cmake)

if(MSVC)
    add_compile_options(/WX /wd4267 /wd4100 /wd4101 /wd4189)
else()
    add_compile_options(-Wno-multichar -Wno-undefined-var-template)
endif()

if(APPLE)
    add_compile_definitions(GL_SILENCE_DEPRECATION)
endif()

# Set where the binary files will be built.  The program will not execute from
# here.  You must run "make install" to install these to the proper location
# as defined above.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Directly integrate libcommon and block install targets
set(LIBCOMMON_GENERATE_INSTALL_TARGETS OFF CACHE BOOL "Make libcommon install targets" FORCE)
set(CODEGEN_GENERATE_INSTALL_TARGETS OFF CACHE BOOL "Make codegen install targets" FORCE)
set(CODEGEN_BUILD_PACKAGE_DURING_CONFIGURE ON)
add_subdirectory(externals/LibCommon)

add_subdirectory(src/Core)
add_subdirectory(src/Editor)
