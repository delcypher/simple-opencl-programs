add_executable( run_kernel run_kernel.c)
target_link_libraries( run_kernel clprobe ${OPENCL_LIBRARIES} )
