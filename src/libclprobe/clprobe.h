#include <CL/opencl.h>
#ifdef __cplusplus
extern "C" {
#endif

/*! Retrieve platform IDs. If Successful the client is responsible for freeing
 *  the memory allocated.
 *
 *  \param[in,out] platforms will be set to point to a list of cl_platforms
 *  \param[in,out] numberOfPlatforms will be set the memory pointed to by this pointer
 *         to the number of OpenCL platforms found.
 *
 *  \returns CL_SUCESS on success.
 */
cl_int getPlatformIDs(cl_platform_id** platforms, cl_uint* numberOfPlatforms);

cl_int printPlatformInfo(cl_platform_id platform, cl_uint indent);

cl_int getDeviceIDs(cl_platform_id platform, cl_device_id** devices, cl_uint* numberOfDevices);

cl_int printDeviceInfo(cl_device_id did, cl_uint indent);
#ifdef __cplusplus
}
#endif
