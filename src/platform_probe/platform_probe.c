#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <CL/opencl.h>

int _handleError(cl_int error, const char* msg)
{
    if ( error != CL_SUCCESS )
    {
        printf("%s\n",msg);
        return 1;
    }
    else
        return 0;
}

// Helper macro
#define handleError(EC,MSG) if (_handleError(EC,MSG) != 0) return 1


int printDI_cstring(cl_device_id did, cl_device_info info)
{
    size_t stringSize=0;
    cl_int err=0;
    err = clGetDeviceInfo(did,
                          info,
                          0,
                          NULL,
                          &stringSize
                         );

    handleError(err, "Couldn't determine string size");

    if (stringSize < 1)
    {
        printf("Error: String size cannot be < 1\n");
        return 1;
    }

    char* str = (char*) malloc(sizeof(char)*stringSize);

    if (str == 0)
    {
        printf("Failed to malloc.\n");
        return 1;
    }

    err = clGetDeviceInfo(did,
                          info,
                          stringSize,
                          str,
                          NULL
                         );

    handleError(err,"Failed to get property info.");

    printf("%s\n",str);
    free(str);
    return 0;
}

int printDI_DeviceType(cl_device_id did, cl_device_info info)
{
    assert( info == CL_DEVICE_TYPE);
    cl_device_type devType=0;
    cl_uint err;
    err = clGetDeviceInfo(did,
                          info,
                          sizeof(devType),
                          &devType,
                          0);

    handleError(err, "Couldn't get device info.");

    // Now handle flags
    #define CHK_FLAG(A) if ( devType & A ) printf(#A " ")
    CHK_FLAG(CL_DEVICE_TYPE_CPU);
    CHK_FLAG(CL_DEVICE_TYPE_GPU);
    CHK_FLAG(CL_DEVICE_TYPE_ACCELERATOR);
    CHK_FLAG(CL_DEVICE_TYPE_DEFAULT);
    #ifdef CL_VERSION_1_2
    CHK_FLAG(CL_DEVICE_TYPE_CUSTOM);
    #endif
    #undef CHK_FLAG

    printf("\n");
    return 0;
}

int printDI_FPflag(cl_device_id did, cl_device_info info)
{
    assert( ( info == CL_DEVICE_SINGLE_FP_CONFIG ||
            info == CL_DEVICE_DOUBLE_FP_CONFIG ) &&
            "Invalid cl_device_info passed.");
    cl_uint err;
    cl_device_fp_config fpConfig;
    err = clGetDeviceInfo( did,
                           info,
                           sizeof(fpConfig),
                           &fpConfig,
                           0
                         );

    handleError(err, "Could not get floating point information\n");

    // Handle flags
    #define CHK_FLAG(A) if (fpConfig & A) printf(#A " ")
    CHK_FLAG(CL_FP_DENORM);
    CHK_FLAG(CL_FP_INF_NAN);
    CHK_FLAG(CL_FP_ROUND_TO_NEAREST);
    CHK_FLAG(CL_FP_ROUND_TO_ZERO);
    CHK_FLAG(CL_FP_ROUND_TO_INF);
    CHK_FLAG(CL_FP_FMA);
    CHK_FLAG(CL_FP_SOFT_FLOAT);

    #ifdef CL_VERSION_1_2
    if ( info == CL_DEVICE_SINGLE_FP_CONFIG )
    {
        CHK_FLAG(CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT);
    }
    #endif
    #undef CHK_FLAG
    printf("\n");
    return 0;
}

void printDeviceInfo(cl_device_id did)
{
    typedef struct
    {
        cl_device_info param;
        const char* name;
        int (*handler) (cl_device_id, cl_device_info);
    } DevicePropTriple;
    #define DEVINFO(A,TYPE) { A, #A, & printDI_ ##TYPE }

    DevicePropTriple dInfos[] = 
    {
        DEVINFO(CL_DEVICE_NAME,cstring),
        DEVINFO(CL_DEVICE_VENDOR,cstring),
        DEVINFO(CL_DRIVER_VERSION,cstring),
        DEVINFO(CL_DEVICE_VERSION,cstring),
        DEVINFO(CL_DEVICE_TYPE,DeviceType),
        DEVINFO(CL_DEVICE_EXTENSIONS,cstring),
        DEVINFO(CL_DEVICE_SINGLE_FP_CONFIG,FPflag),
        DEVINFO(CL_DEVICE_DOUBLE_FP_CONFIG,FPflag)
    };
    #undef DEVINFO
    /* Iterate through properties of interest */
    unsigned int index=0;
    for(; index < sizeof(dInfos)/sizeof(DevicePropTriple); ++index)
    {
        printf("    " /* Indent */ 
               "%s" /* Parameter name */
               ": " /* seperator */
               , dInfos[index].name);

        /* Call the handler for the type to print the information.
           We assume the handler will also print the '\n' character.
        */
        (dInfos[index].handler)(did, dInfos[index].param);
    }
}

int main()
{
    cl_int err = 0;

    cl_uint numOfPlatforms=0;
    err = clGetPlatformIDs(0, NULL, &numOfPlatforms);
    handleError(err, "Failed to determine number of platforms available.");

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

    /* Map id and string name together*/
    typedef struct
    {
        cl_platform_info id;
        const char* name;
    } PlatformInfoPair;

    #define PINFO(A) { A , #A }
    PlatformInfoPair pInfos[] =
    {
        PINFO(CL_PLATFORM_PROFILE),
        PINFO(CL_PLATFORM_VERSION),
        PINFO(CL_PLATFORM_NAME),
        PINFO(CL_PLATFORM_VENDOR),
        PINFO(CL_PLATFORM_EXTENSIONS)
    };
    #undef PINFO

    /* Iterate through platforms */
    unsigned int index=0;
    for( ; index < numOfPlatforms; ++index)
    {
        // Iterate through platform properties
        unsigned int pI=0;
        for(; pI < sizeof(pInfos)/sizeof(PlatformInfoPair); ++pI)
        {
         size_t stringSize;
         char* info;
         err = clGetPlatformInfo( platforms[index],
                                  pInfos[pI].id,
                                  0,
                                  NULL,
                                  &stringSize
                                );
         handleError(err, "Problem querying platform property");

         if (stringSize < 1)
         {
            printf("Bad malloc size, skiping property:%s\n", pInfos[pI].name);
            continue;
         }

         info = (char*) malloc(stringSize * sizeof(char));
         if (info == 0)
         {
            printf("Failed to malloc\n");
            continue;
         }

         err = clGetPlatformInfo( platforms[index],
                                  pInfos[pI].id,
                                  stringSize,
                                  info,
                                  0
                                );
         handleError(err,"Got string size but couldn't get string querying platform info.");
         printf("%s: %s\n", pInfos[pI].name, info);
         free(info);


        }

        /* Find devices for platform*/
        cl_uint numDevices = 0;
        err = clGetDeviceIDs( platforms[index],
                              CL_DEVICE_TYPE_ALL,
                              0,
                              NULL,
                              &numDevices
                            );
        handleError(err, "Couldn't get number of devices for platform.");

        if ( numDevices < 1)
        {
        printf("Platform does not have any devices!\n");
        continue;
        }

        printf("# of devices: %u\n", numDevices);

        cl_device_id* devices = (cl_device_id*) malloc(sizeof(cl_device_id) * numDevices);
        if ( devices == 0 )
        {
            printf("Failed to malloc\n");
            continue;
        }

        err = clGetDeviceIDs( platforms[index],
                              CL_DEVICE_TYPE_ALL,
                              numDevices,
                              devices,
                              0
                            );
        handleError(err, "Could not get devices IDs");

        /* Iterate through devices for this platform*/
        unsigned int deviceIndex=0;
        for (; deviceIndex < numDevices; ++deviceIndex)
        {
            printf("  Device :%u\n", deviceIndex);
            printDeviceInfo( devices[deviceIndex]);
        }

    }

    free(platforms);

    return 0;
}
