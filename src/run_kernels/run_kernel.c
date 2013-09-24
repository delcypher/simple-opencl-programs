#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <CL/opencl.h>
#include <libclprobe/clprobe.h>

void showError(const char* msg)
{
    printf("Error: %s\n", msg);
    exit(1);
}

void handleError(cl_int error, const char* msg)
{
    if ( error != CL_SUCCESS)
    {
        showError(msg);
        assert(0 && "Unreachable");
    }
}

cl_platform_id pickPlatform()
{
    cl_uint numPlatforms=0;
    cl_int err = clGetPlatformIDs(0, 0, &numPlatforms);
    handleError(err, "Could not get number of platforms");

    if ( numPlatforms < 1 )
        showError("Couldn't find a platform");

    cl_platform_id* platforms=0;
    cl_uint numberOfPlatforms;
    err = getPlatformIDs(&platforms, &numberOfPlatforms);
    handleError(err, "Could not get platform ID");
    
    // Just pick first platform
    cl_platform_id platform = platforms[0];
    free(platforms);
    return platform;
}

cl_device_id pickDevice(cl_platform_id platform)
{
    cl_device_id* devices;
    cl_uint numberOfDevices;

    cl_int err = getDeviceIDs(platform, &devices, &numberOfDevices);

    handleError(err, "Could not get device IDs");

    // Just pick first device
    cl_device_id device = devices[0];
    free(devices);
    return device;
}

int main(int argc, char** argv)
{
    cl_platform_id platform = pickPlatform();
    printf("Selected Platform:\n");
    printPlatformInfo(platform,0);
    printf("\n");
    cl_device_id device = pickDevice(platform);
    printf("Selected Device:\n");
    printDeviceInfo(device, 0);
    printf("\n");
    return 0;
}
