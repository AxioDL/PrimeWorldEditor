cmake_minimum_required(VERSION 3.12)

# TODO: These should be drive by CI if CI ever becomes a thing for PWE.
set(PWE_APP_NAME "Prime World Editor")

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
set(CMAKE_AUTORCC ON)

if (WIN32)
    set(QT_PLATFORM_COMPONENTS WinExtras)
    set(QT_PLATFORM_COMPONENTS_LIBS Qt5::WinExtras)
endif()

find_package(Qt5 COMPONENTS Core Widgets ${QT_PLATFORM_COMPONENTS} REQUIRED)

file(GLOB_RECURSE source_files
    "*.cpp"
    "*.h"
    "*.hpp"
    "*.ui"
)

string(TIMESTAMP CURRENT_YEAR "%Y")

if (WIN32)
    generate_product_version(rc_file NAME ${PWE_APP_NAME}
            ICON ${CMAKE_CURRENT_SOURCE_DIR}/icons/win/AppIcon.ico
            VERSION_MAJOR ${pwe_editor_VERSION_MAJOR} VERSION_MINOR ${pwe_editor_VERSION_MINOR}
            VERSION_PATCH ${pwe_editor_VERSION_PATCH} VERSION_REVISION ${pwe_editor_VERSION_TWEAK}
            COMPANY_NAME "AxioDL" COMMENTS "${PWE_APP_NAME} ${PWE_WC_DESCRIBE}"
            ORIGINAL_FILENAME "PrimeWorldEditor.exe")
    set(PLAT_SRCS ${rc_file} icons/win/PrimeWorldEditor.manifest)
elseif (APPLE)
    set_source_files_properties(icons/mac/mainicon.icns PROPERTIES
                                MACOSX_PACKAGE_LOCATION Resources)
    configure_file(icons/mac/Info.plist.in "${CMAKE_CURRENT_BINARY_DIR}/Info.plist" @ONLY)
    file(GLOB_RECURSE RES_SOURCES ${CMAKE_SOURCE_DIR}/resources/* ${CMAKE_SOURCE_DIR}/templates/*)
    foreach(src ${RES_SOURCES})
        file(RELATIVE_PATH relsrc ${CMAKE_SOURCE_DIR} ${src})
        get_filename_component(reldir ${relsrc} DIRECTORY)
        set_source_files_properties(${src} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/${reldir})
    endforeach()
    set(PLAT_SRCS icons/mac/mainicon.icns ${RES_SOURCES} MacOSExtras.mm)
    find_library(APPKIT_LIBRARY AppKit)
    set(PLAT_LIBS ${APPKIT_LIBRARY})
else()
    configure_file(icons/freedesktop/metainfo/io.github.arukibree.PrimeWorldEditor.appdata.xml.in
                   "${CMAKE_CURRENT_BINARY_DIR}/io.github.arukibree.PrimeWorldEditor.appdata.xml" @ONLY)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/io.github.arukibree.PrimeWorldEditor.appdata.xml" DESTINATION share/metainfo)
    install(DIRECTORY icons/freedesktop/applications icons/freedesktop/icons DESTINATION share)
endif()

add_executable(pwe_editor WIN32 MACOSX_BUNDLE ${source_files} Icons.qrc ${PLAT_SRCS})

set_target_properties(pwe_editor PROPERTIES OUTPUT_NAME PrimeWorldEditor DEBUG_POSTFIX -debug
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/Info.plist"
        MACOSX_BUNDLE_BUNDLE_NAME "PrimeWorldEditor")

target_compile_features(pwe_editor PRIVATE cxx_std_17)

target_link_libraries(
    pwe_editor
    pwe_core
    Qt5::Widgets
    ${QT_PLATFORM_COMPONENTS_LIBS}
    ${PLAT_LIBS}
)

target_compile_definitions(
    pwe_editor
    PRIVATE
        "APP_NAME=\"${PWE_APP_NAME}\""
        "APP_VERSION=\"${PROJECT_VERSION}\""
        "APP_FULL_NAME=\"${PWE_APP_NAME} ${PWE_WC_DESCRIBE}\""
        UNICODE
)

gather_include_directories(pwe_editor_include_directories pwe_editor)

add_codegen_targets(
    "${source_files}"
    codegen_generated_files
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_BINARY_DIR}"
    "${pwe_editor_include_directories}"
)
add_custom_target(pwe_editor_codegen DEPENDS ${codegen_generated_files} SOURCES ${source_files})

# Add the generated sources to the library target
target_sources(pwe_editor PRIVATE ${codegen_generated_files})

if (WIN32 OR APPLE)
    set(CPACK_GENERATOR ZIP)
    set(CPACK_STRIP_FILES ON)
    set(CPACK_PACKAGE_VERSION_MAJOR ${pwe_editor_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${pwe_editor_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${pwe_editor_VERSION_PATCH})
    if (APPLE)
        set(CPACK_SYSTEM_NAME macOS)
        set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
    endif()
    if (CMAKE_BUILD_TYPE STREQUAL Debug)
        set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME}-debug)
    endif()
    include(CPack)
endif()

if (WIN32)
    find_program(WINDEPLOYQT_PROGRAM windeployqt PATHS ${Qt5_DIR}/../../../bin/)
    if(WINDEPLOYQT_PROGRAM)
        message(STATUS "Found ${WINDEPLOYQT_PROGRAM}")
    else()
        message(FATAL_ERROR "Unable to find windeployqt")
    endif()

    # run windeployqt to gather necessary qt libraries and plugins
    add_custom_command(TARGET pwe_editor POST_BUILD COMMAND ${WINDEPLOYQT_PROGRAM} ARGS
                       --no-angle --no-opengl-sw $<TARGET_FILE:pwe_editor>)

    # copy bin directory of dew prefix
    add_custom_command(TARGET pwe_editor POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory
                       "${DEW_CMAKE_PREFIX_PATH}/bin" $<TARGET_FILE_DIR:pwe_editor>)

    # Install targets for CPack distribution
    install(DIRECTORY $<TARGET_FILE_DIR:pwe_editor> DESTINATION ".")
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/resources" DESTINATION ".")
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/templates" DESTINATION ".")
elseif (APPLE)
    find_program(MACDEPLOYQT_PROGRAM macdeployqt PATHS ${Qt5_DIR}/../../../bin/)
    if(MACDEPLOYQT_PROGRAM)
        message(STATUS "Found ${MACDEPLOYQT_PROGRAM}")
    else()
        message(FATAL_ERROR "Unable to find macdeployqt")
    endif()

    # run macdeployqt to gather necessary qt libraries and plugins
    option(USE_MACDEPLOYQT "Run macdeployqt on build" OFF)
    if(USE_MACDEPLOYQT)
        add_custom_command(TARGET pwe_editor POST_BUILD COMMAND ${MACDEPLOYQT_PROGRAM} ARGS
                           $<TARGET_FILE_DIR:pwe_editor>/../..)
    endif()

    # Install targets for CPack distribution
    install(TARGETS pwe_editor BUNDLE DESTINATION ".")
else()
    target_compile_definitions(
        pwe_editor
        PRIVATE
            "PWE_DATADIR=\"${CMAKE_INSTALL_PREFIX}/share/PrimeWorldEditor\""
    )

    install(TARGETS pwe_editor)
    set(USER_PERMISSIONS DIRECTORY_PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_WRITE WORLD_EXECUTE
            FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ WORLD_WRITE)
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/resources" DESTINATION share/PrimeWorldEditor ${USER_PERMISSIONS})
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/templates" DESTINATION share/PrimeWorldEditor ${USER_PERMISSIONS})
endif()

if (NOT APPLE)
    add_custom_command(TARGET pwe_editor POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory
                       "${CMAKE_SOURCE_DIR}/resources" "${CMAKE_BINARY_DIR}/resources")

    add_custom_command(TARGET pwe_editor POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory
                       "${CMAKE_SOURCE_DIR}/templates" "${CMAKE_BINARY_DIR}/templates")
endif()
