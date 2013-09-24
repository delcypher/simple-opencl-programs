/* */
#include <clprobe.h>
#include <cstdio>
#include <cstdlib>
#include <cassert>

cl_int getPlatformIDs(cl_platform_id** platforms, cl_uint* numberOfPlatforms)
{
    cl_int err = clGetPlatformIDs(0, NULL, numberOfPlatforms);
    if ( err != CL_SUCCESS )
    {
        printf("Failed to determine number of platforms available.\n");
        return err;
    }


    if ( (*numberOfPlatforms) < 1 )
    {
        printf("The number of platforms available is < 1\n");
        return CL_INVALID_PLATFORM;
    }
    else
    {
        printf("Found %u platform(s)\n", *numberOfPlatforms);
    }

    /* Allocate memory for platform IDs */
    *platforms = (cl_platform_id*) malloc( sizeof(cl_platform_id)* (*numberOfPlatforms));
    if ( platforms == 0 )
    {
        printf("Failed to malloc\n");
        return CL_OUT_OF_HOST_MEMORY;
    }

    err = clGetPlatformIDs(*numberOfPlatforms, *platforms, NULL);
    if ( err != CL_SUCCESS )
    {
       printf("Failed to get platform IDs\n");
       free(platforms);
       return err;
    }

    return CL_SUCCESS;
}

cl_int printPlatformInfo(cl_platform_id platform, cl_uint indent)
{
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

    cl_int err;

    // Iterate through platform properties
    unsigned int pI=0;
    cl_int lastError=CL_SUCCESS;
    for(; pI < sizeof(pInfos)/sizeof(PlatformInfoPair); ++pI)
    {
        size_t stringSize;
        char* info;
        err = clGetPlatformInfo( platform,
                              pInfos[pI].id,
                              0,
                              NULL,
                              &stringSize
                            );
        if ( err != CL_SUCCESS )
        {
            printf("Problem querying platform property\n");
            lastError = err;
            continue;
        }

        if (stringSize < 1)
        {
            printf("Bad malloc size, skiping property:%s\n", pInfos[pI].name);
            lastError = CL_INVALID_PROPERTY;
            continue;
        }

        info = (char*) malloc(stringSize * sizeof(char));
        if (info == 0)
        {
            printf("Failed to malloc\n");
            lastError = CL_OUT_OF_HOST_MEMORY;
            continue;
        }

        err = clGetPlatformInfo( platform,
                              pInfos[pI].id,
                              stringSize,
                              info,
                              0
                            );
        if ( err != CL_SUCCESS)
        { 
            printf("Got string size but couldn't get string querying platform info.\n");
            free(info);
            lastError = CL_INVALID_PLATFORM;
            continue;
        }
        
        for (cl_uint i=0; i < indent; ++i) printf(" ");
        printf("%s: %s\n", pInfos[pI].name, info);
        free(info);
    }

    return lastError;
}

cl_int getDeviceIDs(cl_platform_id platform, cl_device_id** devices, cl_uint* numberOfDevices)
{

    cl_int err;
    /* Find devices for platform*/
    err = clGetDeviceIDs( platform,
                          CL_DEVICE_TYPE_ALL,
                          0,
                          NULL,
                          numberOfDevices
                        );
    if ( err != CL_SUCCESS)
    {
        printf("Couldn't get number of devices for platform.\n");
        return err;
    }

    if ( (*numberOfDevices) < 1)
    {
        printf("Platform does not have any devices!\n");
        return CL_DEVICE_NOT_FOUND;
    }

    printf("# of devices: %u\n", (*numberOfDevices));

    *devices = (cl_device_id*) malloc(sizeof(cl_device_id) * (*numberOfDevices));
    if ( devices == 0 )
    {
        printf("Failed to malloc\n");
        return CL_OUT_OF_HOST_MEMORY;
    }

    err = clGetDeviceIDs( platform,
                          CL_DEVICE_TYPE_ALL,
                          (*numberOfDevices),
                          *devices,
                          0
                        );
    if ( err != CL_SUCCESS )
    {
       printf("Could not get devices IDs\n");
       free(*devices);
       return err;
    }

    return CL_SUCCESS;
}

static cl_int printDI_cstring(cl_device_id did, cl_device_info info)
{
    size_t stringSize=0;
    cl_int err=0;
    err = clGetDeviceInfo(did,
                          info,
                          0,
                          NULL,
                          &stringSize
                         );

    if ( err != CL_SUCCESS ) 
    {
        printf("Couldn't determine string size");
        return err;
    }

    if (stringSize < 1)
    {
        printf("Error: String size cannot be < 1");
        return CL_INVALID_VALUE;
    }

    char* str = (char*) malloc(sizeof(char)*stringSize);

    if (str == 0)
    {
        printf("Failed to malloc.");
        return CL_OUT_OF_HOST_MEMORY;
    }

    err = clGetDeviceInfo(did,
                          info,
                          stringSize,
                          str,
                          NULL
                         );

    if ( err != CL_SUCCESS )
    {
        printf("Failed to get property info.");
        return err;
    }

    printf("%s",str);
    free(str);
    return CL_SUCCESS;
}

static cl_int printDI_DeviceType(cl_device_id did, cl_device_info info)
{
    assert( info == CL_DEVICE_TYPE);
    cl_device_type devType=0;
    cl_int err;
    err = clGetDeviceInfo(did,
                          info,
                          sizeof(devType),
                          &devType,
                          0);

    if ( err != CL_SUCCESS)
    { 
        printf("Couldn't get device info.");
        return err;
    }

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

    return CL_SUCCESS;
}

static cl_int printDI_FPflag(cl_device_id did, cl_device_info info)
{
    assert( ( info == CL_DEVICE_SINGLE_FP_CONFIG ||
            info == CL_DEVICE_DOUBLE_FP_CONFIG ) &&
            "Invalid cl_device_info passed.");
    cl_int err;
    cl_device_fp_config fpConfig;
    err = clGetDeviceInfo( did,
                           info,
                           sizeof(fpConfig),
                           &fpConfig,
                           0
                         );

    if ( err != CL_SUCCESS )
    {
        printf("Could not get floating point information");
        return err;
    }

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
    return CL_SUCCESS;
}

static void printT(cl_uint t) { printf("%u",t);}
static void printT(cl_int t) { printf("%d",t); }
static void printT(size_t t) { printf("%lu", (unsigned long) t); }

template <typename T>
static cl_int printDI_t(cl_device_id did, cl_device_info info)
{
    cl_int err;
    T param;
    err = clGetDeviceInfo( did,
                           info,
                           sizeof(T),
                           &param,
                           0
                         );
    if ( err != CL_SUCCESS)
    {
        printf("Could not get information for generic type\n");
        return err;
    }

    printT(param);
    return CL_SUCCESS;
}

/* Cannot use template specialisation here.
*  As cl_bool's underlying type is cl_uint
*  which needs treating with the generic template.
*  So instead just have another special method
*  just for cl_bool types.
*/
static cl_int printDI_cl_bool(cl_device_id did, cl_device_info info)
{
    cl_int err;
    cl_bool result;
    err = clGetDeviceInfo( did,
                           info,
                           sizeof(cl_bool),
                           &result,
                           0
                         );

    if ( err != CL_SUCCESS )
    {
        printf("Could not get cl_bool info");
        return err;
    }

    if (result == CL_TRUE)
        printf("CL_TRUE");
    else if (result == CL_FALSE)
        printf("CL_FALSE");
    else
        printf("FIXME: Unknown!");

    return CL_SUCCESS;
}

static cl_int printDI_workItemSizes(cl_device_id did, cl_device_info info)
{
    assert( info == CL_DEVICE_MAX_WORK_ITEM_SIZES &&
           "Wrong handler");

    // Get number of dimensions (expect 3)
    cl_uint numDim=0;
    cl_int err;
    err = clGetDeviceInfo(did,
                          CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,
                          sizeof(numDim),
                          &numDim,
                          0
                         );
    if ( err != CL_SUCCESS )
    {
        printf("Failed to get CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS\n");
        return err;
    }

    if (numDim < 1 )
    {
        printf("Failed to get number of dimensions\n");
        return CL_INVALID_WORK_DIMENSION;
    }

    size_t* dimMax = (size_t*) malloc( sizeof(size_t)*numDim );
    if (dimMax == 0 )
    {
        printf("Malloc failed.\n");
        return CL_OUT_OF_HOST_MEMORY;
    }

    err = clGetDeviceInfo(did,
                          info,
                          sizeof(size_t) * numDim,
                          dimMax,
                          0
                         );

    if ( err != CL_SUCCESS )
    {
        printf("Failed to get CL_DEVICE_MAX_WORK_ITEM_SIZES\n");
        return err;
    }
    cl_uint d=0;
    printf("[ ");
    for( ; d < numDim ; ++d)
    {
        printf("%lu ", (unsigned long) dimMax[d]);
    }
    printf("]");
    
    free(dimMax);
    return CL_SUCCESS;
}

cl_int printDeviceInfo(cl_device_id did, cl_uint indent)
{
    typedef struct
    {
        cl_device_info param;
        const char* name;
        cl_int (*handler) (cl_device_id, cl_device_info);
    } DevicePropTriple;
    #define DEVINFO(A,TYPE) { A, #A, & printDI_ ##TYPE }

    DevicePropTriple dInfos[] = 
    {
        DEVINFO(CL_DEVICE_NAME, cstring),
        DEVINFO(CL_DEVICE_VENDOR, cstring),
        DEVINFO(CL_DEVICE_VENDOR_ID, t<cl_uint>),
        DEVINFO(CL_DRIVER_VERSION, cstring),
        DEVINFO(CL_DEVICE_VERSION, cstring),
        DEVINFO(CL_DEVICE_OPENCL_C_VERSION, cstring),
        DEVINFO(CL_DEVICE_TYPE, DeviceType),
        DEVINFO(CL_DEVICE_AVAILABLE, cl_bool),
        DEVINFO(CL_DEVICE_COMPILER_AVAILABLE, cl_bool),
        DEVINFO(CL_DEVICE_SINGLE_FP_CONFIG, FPflag),
        DEVINFO(CL_DEVICE_DOUBLE_FP_CONFIG, FPflag),
        DEVINFO(CL_DEVICE_MAX_COMPUTE_UNITS, t<cl_uint>),
        DEVINFO(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, t<cl_uint>),
        DEVINFO(CL_DEVICE_MAX_WORK_GROUP_SIZE, t<size_t>),
        DEVINFO(CL_DEVICE_MAX_WORK_ITEM_SIZES, workItemSizes),
        DEVINFO(CL_DEVICE_MAX_CLOCK_FREQUENCY, t<cl_uint>),  /* In MHz */
        DEVINFO(CL_DEVICE_ADDRESS_BITS, t<cl_uint>),
        DEVINFO(CL_DEVICE_MAX_MEM_ALLOC_SIZE, t<cl_ulong>), /* In bytes */
        DEVINFO(CL_DEVICE_IMAGE_SUPPORT, cl_bool),
        DEVINFO(CL_DEVICE_ENDIAN_LITTLE, cl_bool),
        #ifdef CL_VERSION_1_2
        DEVINFO(CL_DEVICE_LINKER_AVAILABLE, cl_bool),
        DEVINFO(CL_DEVICE_BUILT_IN_KERNELS),
        #endif
        DEVINFO(CL_DEVICE_HOST_UNIFIED_MEMORY, cl_bool),
        DEVINFO(CL_DEVICE_ERROR_CORRECTION_SUPPORT, cl_bool),
        DEVINFO(CL_DEVICE_MAX_PARAMETER_SIZE, t<size_t>), /* In bytes */
        DEVINFO(CL_DEVICE_MEM_BASE_ADDR_ALIGN, t<cl_uint>), /* OpenCL 1.2 Spec is confusing here */
        DEVINFO(CL_DEVICE_GLOBAL_MEM_SIZE, t<cl_ulong>), /* In bytes */
        DEVINFO(CL_DEVICE_LOCAL_MEM_SIZE, t<cl_ulong>), /* In bytes */
        DEVINFO(CL_DEVICE_EXTENSIONS, cstring)
    };
    #undef DEVINFO
    /* Iterate through properties of interest */
    unsigned int index=0;
    cl_int lastError=CL_SUCCESS;
    for(; index < sizeof(dInfos)/sizeof(DevicePropTriple); ++index)
    {
        for (cl_uint i=0; i < indent; ++i) printf(" "); // Do indentation

        printf( "%s" /* Parameter name */
               ": " /* seperator */
               , dInfos[index].name);

        /* Call the handler for the type to print the information.
        */
        lastError = (dInfos[index].handler)(did, dInfos[index].param);

        printf("\n");
    }

    return lastError;
}
