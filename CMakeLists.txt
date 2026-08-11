cmake_minimum_required(VERSION 3.24)
set(MACOSX_DEPLOYMENT_TARGET 10.10)

# Use release build type as default (assimp complains if this is unset)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE})

project(PrimeWorldEditor LANGUAGES C CXX)

option(PWE_PUBLIC_RELEASE "Enable end-user deployment configuration for PWE" ON)
if (PWE_PUBLIC_RELEASE AND "${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    add_compile_definitions(PUBLIC_RELEASE)
    message(STATUS "Enabled public release mode")
endif()

# Ensure submodules checked out
if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/externals/LibCommon/CMakeLists.txt)
    message(FATAL_ERROR "Please run 'git submodule update --init --recursive' to fetch submodules.")
endif()

include(cmake/generate_product_version.cmake)

set(CMAKE_CXX_STANDARD 23)

# Dependencies
include(FetchContent)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
set(ASSIMP_BUILD_MINIZIP ON)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_BUILD_ZLIB OFF)
set(ASSIMP_WARNINGS_AS_ERRORS OFF)
set(tinyxml2_BUILD_TESTING OFF)
set(BUILD_SHARED_LIBS OFF)

set(ZLIB_BUILD_EXAMPLES OFF)
set(ZLIB_LIBRARY zlibstatic)
FetchContent_Declare(
  zlib
  GIT_REPOSITORY https://github.com/madler/zlib.git
  GIT_TAG        v1.3.1
  OVERRIDE_FIND_PACKAGE
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(zlib)
set(ZLIB_INCLUDE_DIR ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        12.2.0
  OVERRIDE_FIND_PACKAGE
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(fmt)

set(SPDLOG_FMT_EXTERNAL ON)
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.17.0
  OVERRIDE_FIND_PACKAGE
  EXCLUDE_FROM_ALL
)
FetchContent_Declare(
  assimp
  GIT_REPOSITORY https://github.com/assimp/assimp.git
  GIT_TAG        v6.0.2
  OVERRIDE_FIND_PACKAGE
  EXCLUDE_FROM_ALL
)
FetchContent_Declare(
  nod
  GIT_REPOSITORY https://github.com/AxioDL/nod.git
  GIT_TAG        20fa7905b99a81d335769459ee8f6d7b81c923c2
  OVERRIDE_FIND_PACKAGE
  EXCLUDE_FROM_ALL
)
FetchContent_Declare(
  lzokay
  GIT_REPOSITORY https://github.com/AxioDL/lzokay.git
  GIT_TAG        db2df1fcbebc2ed06c10f727f72567d40f06a2be
  OVERRIDE_FIND_PACKAGE
  EXCLUDE_FROM_ALL
)
FetchContent_Declare(
  tinyxml2
  GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
  GIT_TAG        11.0.0
  OVERRIDE_FIND_PACKAGE
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(spdlog assimp nod lzokay tinyxml2)

if(MSVC)
    # I hate hate hate microsoft compiler devs for not just letting /utf-8 and /source-charset:utf-8 be present
    # alongside one another without errors (assimp requires this).
    get_target_property(EXTLIB_COMPILE_FLAGS assimp COMPILE_OPTIONS)
    list(REMOVE_ITEM EXTLIB_COMPILE_FLAGS /source-charset:utf-8)
    set_target_properties(assimp PROPERTIES COMPILE_OPTIONS "${EXTLIB_COMPILE_FLAGS}")

    add_compile_options(
        /MP     # Multiprocessor compilation

        /WX     # Warnings as errors
        /wd4267 # 'var' : conversion from 'size_t' to 'type', possible loss of data
        /wd4100 # 'identifier' : unreferenced formal parameter
        /wd4101 # 'identifier': unreferenced local variable
        /wd4189 # 'identifier' : local variable is initialized but not referenced

        # Compiler conformance settings
        /permissive-
        /volatile:iso
        /Zc:enumTypes
        /Zc:inline
        /Zc:preprocessor
        /Zc:templateScope
        /Zc:throwingNew
    )
else()
    add_compile_options(-Wno-multichar -Wno-undefined-var-template -Wno-error=array-bounds -fcommon)
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
set(CODEGEN_BUILD_PACKAGE_DURING_CONFIGURE ON)
add_subdirectory(externals/LibCommon)

add_subdirectory(src/Core)
add_subdirectory(src/Editor)
