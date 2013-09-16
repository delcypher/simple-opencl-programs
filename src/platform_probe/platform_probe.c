#include <stdio.h>
#include <stdlib.h>
#include <CL/opencl.h>

int main()
{
    cl_int err = 0;

    cl_uint numOfPlatforms=0;
    err = clGetPlatformIDs(0, NULL, &numOfPlatforms);
    if ( err != CL_SUCCESS )
    {
        printf("Failed to determine number of platforms available.\n");
        return 1;
    }

    if ( numOfPlatforms < 1 )
    {
        printf("The number of platforms available is < 1\n");
        return 1;
    }
    else
    {
        printf("Found %u platform(s)\n", numOfPlatforms);
    }

    /* Allocate memory for platform IDs */
    cl_platform_id* platforms = (cl_platform_id*) malloc( sizeof(cl_platform_id)* numOfPlatforms);
    if ( platforms == 0 )
    {
        printf("Failed to malloc\n");
        return 1;
    }

    err = clGetPlatformIDs(numOfPlatforms, platforms, NULL);
    if ( err != CL_SUCCESS )
    {
       printf("Failed to get platform IDs\n");
       return 1;
    }


    free(platforms);

    return 0;
}
