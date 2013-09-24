#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/stat.h>
#include <CL/opencl.h>
#include <libclprobe/clprobe.h>
#include <errno.h>

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

void contextCallBack(const char* errInfo,
                     const void* privateInfo,
                     size_t cb,
                     void* userData)
{
    static unsigned int count=0;
    count++;
    printf("Context Error (#%u): %s\n", count, errInfo);
}

void usage(const char* progName)
{
    printf("Usage: %s <kernel file>\n", progName);
    exit(1);
}

/* The Client is responsible for freeing the memory
*  allocated.
*
*  Returns a NULL pointer if file cannot be opened.
*/
char* loadKernelFromFile(const char* path)
{
    char* source=0;

    struct stat fileInfo;
    if( stat(path, &fileInfo) !=  0)
    {
        perror("Could not access file");
        return NULL;
    }

    /* File size in bytes */
    off_t fileSize = fileInfo.st_size;
    if ( fileSize < 1 )
    {
        printf("Reported file size of %ld bytes is invalid.\n", fileSize);
        return NULL;
    }

    FILE* f = fopen(path,"rb");

    if (f == NULL)
    {
        perror("Failed to open file.");
        return NULL;
    }
    
    // Allocated memory for file
    source = (char*) malloc(  fileSize + 
                            /* For '\0' terminator */ 1);

    if (source == 0)
    {
        printf("Could not allocated memory for kernel file.");
        fclose(f);
        return NULL;
    }

    if ( fread(/*ptr*/ source, /* no of bytes */ 1 , fileSize , /*FILE*/ f) != fileSize)
    {
        printf("Failed to read %s into memory.", path);
        fclose(f);
        free(source);
        return NULL;
    }

    // Write NULL terminator
    source[fileSize] = '\0';

    return source;
}

int main(int argc, char** argv)
{

    if (argc != 2)
    {
        usage(argv[0]);
        assert(0 && "Unreachable");
    }

    char* kernelSource = loadKernelFromFile( argv[1]);
    if (kernelSource == NULL)
    {
        printf("Could not open OpenCL kernel: %s\n", argv[1]);
        exit(1);
    }
    else
    {
        printf("%s loaded as string into memory.\n", argv[1]);
    }



    cl_platform_id platform = pickPlatform();
    printf("Selected Platform:\n");
    printPlatformInfo(platform,0);
    printf("\n");

    cl_device_id device = pickDevice(platform);
    printf("Selected Device:\n");
    printDeviceInfo(device, 0);
    printf("\n");

    /* Create Context */
    cl_int err=CL_SUCCESS;
    cl_context_properties cProp[] = { CL_CONTEXT_PLATFORM, (cl_context_properties) platform, 0 };
    cl_context context= clCreateContext( /*properties*/ cProp,
                                         /*num_device*/ 1,
                                         /*devices*/ &device,
                                         /*CL_CALLBACK*/ contextCallBack,
                                         /*user_data*/ NULL,
                                         &err
                                       );

    if ( err != CL_SUCCESS )
    {
        printf("Failed to create context: %d\n", err);
        return 1;
    }
    else
        printf("Created context.\n");

    printf("Context:\n");
    err = printContextInfo(context,0);
    printf("\n");


    free(kernelSource);
    err = clReleaseContext(context);
    if ( err != CL_SUCCESS )
    {
        printf("Could not release context with error code:%d\n", err);
    }
    return 0;
}
