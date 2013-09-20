#include <stdio.h>
#include <stdlib.h>
#include <CL/opencl.h>

void handleError(cl_int error, const char* msg)
{
    if ( error != CL_SUCCESS )
    {
        printf("%s\n",msg);
        exit(1);
    }
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

    }


    free(platforms);

    return 0;
}
