# Simple CMake project root

cmake_minimum_required( VERSION 2.8 )
project ( OpenCL_examples )

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/findopencl")
find_package( OpenCL REQUIRED )

message(STATUS "Found OpenCL include paths:${OPENCL_INCLUDE_DIRS}")
message(STATUS "Found OpenCL library path:${OPENCL_LIBRARIES}")

include_directories( ${OPENCL_INCLUDE_DIRS} )

# Build examples
subdirs( src )
