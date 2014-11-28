add_executable( prefix_sum prefix_sum.cpp)
target_link_libraries( prefix_sum clprobe ${OPENCL_LIBRARIES} )

set(kernels naive_prefix_sum.cl)

add_custom_target(copyKernels2)

foreach(kernel ${kernels})
    message(STATUS "Found kernel ${kernel}")
    add_custom_command(TARGET copyKernels2 POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E echo Copying kernel ${kernel}
                       COMMAND ${CMAKE_COMMAND} -E
                       copy ${CMAKE_CURRENT_SOURCE_DIR}/${kernel} ${CMAKE_CURRENT_BINARY_DIR}/${kernel})
endforeach()

#Trigger copying of kernel when we build run_kernel
add_dependencies(prefix_sum copyKernels2)
