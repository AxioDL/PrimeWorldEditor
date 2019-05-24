cmake_minimum_required(VERSION 3.12)

# obtain revision info from git
find_package(Git)
if(GIT_FOUND)
    # make sure version information gets re-run when the current Git HEAD changes
    execute_process(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} COMMAND ${GIT_EXECUTABLE} rev-parse --git-path HEAD
        OUTPUT_VARIABLE pwe_git_head_filename
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${pwe_git_head_filename}")

    execute_process(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} COMMAND ${GIT_EXECUTABLE} rev-parse --symbolic-full-name HEAD
        OUTPUT_VARIABLE pwe_git_head_symbolic
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMAND ${GIT_EXECUTABLE} rev-parse --git-path ${pwe_git_head_symbolic}
        OUTPUT_VARIABLE pwe_git_head_symbolic_filename
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${pwe_git_head_symbolic_filename}")

    # defines PWE_WC_REVISION
    execute_process(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
        OUTPUT_VARIABLE PWE_WC_REVISION
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    # defines PWE_WC_DESCRIBE
    execute_process(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} COMMAND ${GIT_EXECUTABLE} describe --always --long --dirty
        OUTPUT_VARIABLE PWE_WC_DESCRIBE
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    # remove hash (and trailing "-0" if needed) from description
    string(REGEX REPLACE "(-0)?-[^-]+((-dirty)?)$" "\\2" PWE_WC_DESCRIBE "${PWE_WC_DESCRIBE}")

    # defines PWE_WC_BRANCH
    execute_process(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
        OUTPUT_VARIABLE PWE_WC_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

if (PWE_WC_DESCRIBE)
    string(REGEX REPLACE "v([0-9]+)\.([0-9]+)\.([0-9]+)\-([0-9]+).*" "\\1.\\2.\\3.\\4" PWE_VERSION_STRING "${PWE_WC_DESCRIBE}")
    string(REGEX REPLACE "v([0-9]+)\.([0-9]+)\.([0-9]+).*" "\\1.\\2.\\3" PWE_VERSION_STRING "${PWE_VERSION_STRING}")
else()
    set(PWE_WC_DESCRIBE "UNKNOWN-VERSION")
    set(PWE_VERSION_STRING "0.0.0")
endif()

project(pwe_editor LANGUAGES CXX VERSION ${PWE_VERSION_STRING})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if (WIN32)
    set(QT_PLATFORM_COMPONENTS WinExtras)
    set(QT_PLATFORM_COMPONENTS_LIBS Qt5::WinExtras)
endif()

find_package(Qt5 COMPONENTS Core Widgets ${QT_PLATFORM_COMPONENTS} REQUIRED)
find_package(assimp CONFIG REQUIRED)

# AssImp's cmake config is pretty awful. It doesn't include necesary libraries. Hopefully this can be fixed later.
find_library(IIRXML_LIBRARY NAMES IrrXMLd IrrXML)

file(GLOB_RECURSE source_files
    "*.cpp"
    "*.h"
    "*.hpp"
)

add_executable(pwe_editor ${source_files})

set_target_properties(pwe_editor PROPERTIES OUTPUT_NAME PrimeWorldEditor DEBUG_POSTFIX -debug)

target_compile_features(pwe_editor PRIVATE cxx_std_17)

target_link_libraries(
    pwe_editor
    pwe_core
    Qt5::Widgets
    ${QT_PLATFORM_COMPONENTS_LIBS}
    ${IIRXML_LIBRARY}
)

# TODO: These should be drive by CI if CI ever becomes a thing for PWE.
set(PWE_APP_NAME "Prime World Editor")

target_compile_definitions(
    pwe_editor
    PRIVATE
        "APP_NAME=\"${PWE_APP_NAME}\""
        "APP_VERSION=\"${PROJECT_VERSION}\""
        "PUBLIC_RELEASE=1"
        "APP_FULL_NAME=\"${PWE_APP_NAME} ${PWE_WC_DESCRIBE}\""
        UNICODE
)

if (WIN32)
    find_program(WINDEPLOYQT_PROGRAM windeployqt PATHS $ENV{QTDIR}/bin/)
    if(WINDEPLOYQT_PROGRAM)
        message(STATUS "Found ${WINDEPLOYQT_PROGRAM}")
    else()
        message(FATAL_ERROR "Unable to find windeployqt")
    endif()

    # run windeployqt to gather necessary .dlls
    add_custom_command(TARGET pwe_editor POST_BUILD COMMAND ${WINDEPLOYQT_PROGRAM} ARGS
                       --no-angle --no-opengl-sw $<TARGET_FILE:pwe_editor>)

    # copy bin directory of dew prefix
    add_custom_command(TARGET pwe_editor POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory
                       "${DEW_CMAKE_PREFIX_PATH}/bin" $<TARGET_FILE_DIR:pwe_editor>)
endif()
