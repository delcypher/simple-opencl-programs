#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/stat.h>
#include <CL/opencl.h>
#include <libclprobe/clprobe.h>
#include <errno.h>

void showError(const char* msg, bool quit=true)
{
    printf("Error: %s\n", msg);
    if (quit) exit(1);
}

void handleError(cl_int error, const char* msg, bool quit=true)
{
    if ( error != CL_SUCCESS)
    {
        showError(msg, quit);
        if (quit) assert(0 && "Unreachable");
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

    if ( fread(/*ptr*/ source, /* no of bytes */ 1 , fileSize , /*FILE*/ f) != ( (size_t) fileSize) )
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


void cleanUp();

//Global for clean up convenience
char* kernelSource=0;
cl_program program=0;
cl_context context=0;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        usage(argv[0]);
        assert(0 && "Unreachable");
    }

    kernelSource = loadKernelFromFile( argv[1]);
    if (kernelSource == NULL)
    {
        printf("Could not open OpenCL kernel: %s\n", argv[1]);
        exit(1);
    }
    else
    {
        printf("%s loaded as string into memory.\n", argv[1]);
    }

    cl_platform_id platform=0;
    cl_device_id device=0;
    cl_int err=CL_SUCCESS;

    platform = pickPlatform();
    printf("Selected Platform:\n");
    printPlatformInfo(platform,0);
    printf("\n");

    device = pickDevice(platform);
    printf("Selected Device:\n");
    printDeviceInfo(device, 0);
    printf("\n");

    /* Create Context */
    cl_context_properties cProp[] = { CL_CONTEXT_PLATFORM, (cl_context_properties) platform, 0 };
    context= clCreateContext( /*properties*/ cProp,
                              /*num_device*/ 1,
                              /*devices*/ &device,
                              /*CL_CALLBACK*/ contextCallBack,
                              /*user_data*/ NULL,
                              &err
                            );

    if ( err != CL_SUCCESS )
    {
        printf("Failed to create context: %d\n", err);
        cleanUp();
        exit(1);
    }
    else
        printf("Created context.\n");

    printf("Context:\n");
    err = printContextInfo(context,0);
    printf("\n");

    /* Create kernel */
    program = clCreateProgramWithSource( context, 
                                         /*number of strings*/ 1,
                                         (const char**)  &kernelSource,
                                         /* NULL terminated, don't need lengths*/ NULL,
                                         &err
                                       );

    if ( err != CL_SUCCESS )
    {
        printf("Could not create program:%d\n", err);
        cleanUp();
        exit(1);
    }

    /* Compile Kernel */
    printf("Trying to compile & link kernel.\n");
    err = clBuildProgram( program, 
                          /* num_devices */ 1,
                          /* devices*/ &device,
                          /* Compiler options */ NULL,
                          /* Callback */ NULL,
                          /* User Data for call back */ NULL
                        );

    // Output build log
    printProgramBuildInfo(program, device, /*Indent*/ 0);
    if ( err != CL_SUCCESS )
    {
        printf("Build failed\n");
        cleanUp();
        exit(1);
    }

    printProgramInfo(program, /*Indent*/ 0);

    cleanUp();
    return 0;
}

void cleanUp()
{
    cl_int err=0;
    free(kernelSource);

    if (program!= 0)
    {
        err = clReleaseProgram(program);
        handleError(err, "Couldn't release program", true);
    }

    if (context!=0)
    {
        err = clReleaseContext(context);
        handleError(err, "Couldn't release context", true);
    }
}
