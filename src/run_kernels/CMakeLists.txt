add_executable( run_kernel run_kernel.cpp)
target_link_libraries( run_kernel clprobe ${OPENCL_LIBRARIES} )

set(kernels add.cl dot_product.cl)

add_custom_target(copyKernels)

foreach(kernel ${kernels})
    message(STATUS "Found kernel ${kernel}")
    add_custom_command(TARGET copyKernels POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E echo Copying kernel ${kernel}
                       COMMAND ${CMAKE_COMMAND} -E
                       copy ${CMAKE_CURRENT_SOURCE_DIR}/${kernel} ${CMAKE_CURRENT_BINARY_DIR}/${kernel})
endforeach()

#Trigger copying of kernel when we build run_kernel
add_dependencies(run_kernel copyKernels)
