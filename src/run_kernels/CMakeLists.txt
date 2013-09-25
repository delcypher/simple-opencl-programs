add_executable( run_kernel run_kernel.cpp)
target_link_libraries( run_kernel clprobe ${OPENCL_LIBRARIES} )
