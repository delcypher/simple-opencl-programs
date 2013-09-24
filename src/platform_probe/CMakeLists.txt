add_executable( platform_probe platform_probe.cpp )
target_link_libraries( platform_probe clprobe ${OPENCL_LIBRARIES} )
