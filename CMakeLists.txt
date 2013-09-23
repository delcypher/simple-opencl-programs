# Simple CMake project root

cmake_minimum_required( VERSION 2.8 )
project ( OpenCL_examples )

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/findopencl")
find_package( OpenCL REQUIRED )

message(STATUS "Found OpenCL include paths:${OPENCL_INCLUDE_DIRS}")
message(STATUS "Found OpenCL library path:${OPENCL_LIBRARIES}")

include_directories( ${OPENCL_INCLUDE_DIRS} )

if(NOT CMAKE_BUILD_TYPE)
    # Force a default build type
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type: Release, RelWithDebInfo, Debug or MinSizeRel" FORCE)
endif()

# Provide drop down menu options in cmake-gui
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Release RelWithDebInfo Debug MinSizeRel)

# Force warnings
if(CMAKE_C_COMPILER_ID MATCHES "(Clang|GNU)")
    message(STATUS "Enabling compiler warnings")
    add_definitions(-Wall)
else()
    message(WARNING "FIXME:Could not enable compiler warnings.")
endif()

# Build examples
subdirs( src )
