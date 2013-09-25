add_executable( run_kernel run_kernel.cpp)
target_link_libraries( run_kernel clprobe ${OPENCL_LIBRARIES} )

set(kernels add.cl)

foreach(kernel ${kernels})
    # Use configure_file so that if kernel changes file will be copied over
    message(STATUS "Copying kernel ${kernel}")
    configure_file(${kernel} ${CMAKE_CURRENT_BINARY_DIR}/${kernel} COPYONLY)
endforeach()
