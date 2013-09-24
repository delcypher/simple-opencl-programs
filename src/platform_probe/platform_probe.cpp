#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <CL/opencl.h>
#include <libclprobe/clprobe.h>

int main(int argc, char** argv)
{
    const cl_uint indent=2;
    cl_int err = 0;
    cl_platform_id* platforms;
    cl_uint numOfPlatforms;
    err = getPlatformIDs( &platforms, &numOfPlatforms);
    if ( err != CL_SUCCESS )
    {
        printf("Failed to get platformsIDs\n");
        exit(1);
    }

    /* Iterate through platforms */
    unsigned int index=0;
    for( ; index < numOfPlatforms; ++index)
    {
        printf("Platform # %u\n", index);

        err = printPlatformInfo( platforms[index], indent);
        if ( err != CL_SUCCESS )
        {
            printf("Failed to print platform info.\n");
            exit(1);
        }
        

        /* Find devices for platform*/
        cl_uint numDevices = 0;
        cl_device_id* devices=0;
        err = getDeviceIDs(platforms[index], &devices, &numDevices);
        if ( err != CL_SUCCESS )
        {
            printf("Failed to get devices IDs\n");
            exit(1);
        }

        /* Iterate through devices for this platform*/
        unsigned int deviceIndex=0;
        for (; deviceIndex < numDevices; ++deviceIndex)
        {
            for (unsigned int i=0; i < indent ; ++i) printf(" ");

            printf("Device :%u\n", deviceIndex);
            printDeviceInfo( devices[deviceIndex], indent*2);
        }

        printf("\n");
        free(devices);

    }

    free(platforms);

    return 0;
}
