cmake_minimum_required(VERSION 3.12)
set(MACOSX_DEPLOYMENT_TARGET 10.10)

project(PrimeWorldEditor CXX)

include(./dew.cmake)
integrate_dew()

include(cmake/generate_product_version.cmake)

# Set where the binary files will be built.  The program will not execute from
# here.  You must run "make install" to install these to the proper location
# as defined above.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src/Core)
add_subdirectory(src/Editor)
