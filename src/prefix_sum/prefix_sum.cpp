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
    printf("Usage: %s <kernel file> <array_size>\n", progName);
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

void printArray(cl_int* array, cl_uint nElements)
{
    for (cl_uint index=0; index < nElements; ++index)
    {
        printf("Array[%u] = %d\n", index, array[index]);
    }
}

//Global for clean up convenience
char* kernelSource=0;
cl_program program=0;
cl_context context=0;
cl_command_queue cmdQueue=0;
cl_kernel kernel=0;
cl_int* hostArrayA=0;
cl_int* hostArrayB=0;
cl_int* copiedBackArray=0;
cl_mem arrayABuffer=0;
cl_mem arrayBBuffer=0;

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        usage(argv[0]);
        assert(0 && "Unreachable");
    }

    kernelSource = loadKernelFromFile( argv[1]);
    unsigned int arraySize = atoi( argv[2] );
    printf("Using array size of %u\n", arraySize);

    // Check is power of 2
    if ( (arraySize & (arraySize -1)) != 0 || arraySize <= 0)
    {
        printf("Array size must be a power of two\n");
        exit(1);
    }

    assert( arraySize > 0 && arraySize < 512 && "Array size too big");

    /* compute number of loop iterations */
    // do log_2(arraySize)
    int numOfIterations = __builtin_ctz(arraySize);
    printf("Computed # of loop iterations: %d\n", numOfIterations);

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

    /* Create command queue */
    cmdQueue = clCreateCommandQueue( context,
                                     device,
                                     /*properties */ 0,
                                     &err
                                   );

    if ( err != CL_SUCCESS )
    {
        printf("Couldn't create command queue.\n");
        cleanUp();
        exit(1);
    }

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

    #ifndef KLEE_CL
    // Output build log
    printProgramBuildInfo(program, device, /*Indent*/ 0);
    #endif
    if ( err != CL_SUCCESS )
    {
        printf("Build failed\n");
        cleanUp();
        exit(1);
    }

    #ifndef KLEE_CL
    printProgramInfo(program, /*Indent*/ 0);
    #endif

    /* Create kernel object */
    kernel = clCreateKernel( program, "prefix_sum", &err);
    if (err != CL_SUCCESS )
    {
        printf("Failed to create kernel object.\n");
        cleanUp();
        exit(1);
    }

    /* Create array to copied to host */
    hostArrayA = (cl_int*) malloc( sizeof(cl_int) * arraySize );
    hostArrayB = (cl_int*) malloc( sizeof(cl_int) * arraySize );
    copiedBackArray = (cl_int*) malloc( sizeof(cl_int) * arraySize );
    if ( hostArrayA == 0 || hostArrayB == 0 )
    {
        printf("Failed to malloc memory for host array\n");
        cleanUp();
        exit(1);
    }

    // fill with sequential values (starting from 1)
    for(cl_uint index=0; index < arraySize; ++index)
    {
        hostArrayA[index] = index +1;
    }

    // Zero the other array
    for(cl_uint index=0; index < arraySize; ++index)
    {
        hostArrayB[index] = 0;
    }

    printf("Created Array:\n");
    printArray( hostArrayA, arraySize);
    printf("\n");
    printArray( hostArrayB, arraySize);
    printf("\n");

    // Create Buffer
    arrayABuffer = clCreateBuffer(context,
                                 CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                 sizeof(cl_int) * arraySize,
                                 hostArrayA,
                                 &err
                                );

    if ( err != CL_SUCCESS )
    {
        printf("Failed to create buffer. Error:%d\n", err);
        cleanUp();
        exit(1);
    }

    arrayBBuffer = clCreateBuffer(context,
                                 CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                 sizeof(cl_int) * arraySize,
                                 hostArrayB,
                                 &err
                                );

    if ( err != CL_SUCCESS )
    {
        printf("Failed to create buffer. Error:%d\n", err);
        cleanUp();
        exit(1);
    }


    /* Setup kernel arguments */
    err |= clSetKernelArg( kernel,
                          /* argument index*/ 0,
                          sizeof(cl_mem),
                          &arrayABuffer
                        );

    err |= clSetKernelArg( kernel,
                          /* argument index*/ 1,
                          sizeof(cl_mem),
                          &arrayBBuffer
                        );

    err |= clSetKernelArg( kernel,
                          /* argument index*/ 2,
                          sizeof(int),
                          &numOfIterations
                        );

    if ( err != CL_SUCCESS )
    {
        printf("Couldn't set kernel argument.\n");
        cleanUp();
        exit(1);
    }

    size_t globalWorkSize[] = { arraySize };
    size_t localWorkSize[] = { arraySize };

    printf("Enquing kernel.\n");
    /* Enqueue kernel */
    err = clEnqueueNDRangeKernel( cmdQueue,
                                  kernel,
                                  /* Work dim */ 1,
                                  /* global_work_offset */ NULL,
                                  /* global_work_size */ globalWorkSize,
                                  /* local_work_size */ localWorkSize,
                                  /* num_events_in_wait_list */ 0,
                                  /* event_wait_list */ NULL,
                                  /* event */ NULL
                                 );

    if ( err != CL_SUCCESS )
    {
        printf("Failed to enqueue kernel.\n");
        cleanUp();
        exit(1);
    }

    /* Read back array */
    cl_mem resultBuffer = (numOfIterations % 2 != 0)? arrayBBuffer: arrayABuffer;
    err = clEnqueueReadBuffer( cmdQueue,
                               resultBuffer,
                               /* blocking_read */ CL_TRUE,
                               /* offset */ 0,
                               /* size */ sizeof(cl_int)*arraySize,
                               copiedBackArray,
                               /* num_events_in_wait_list */ 0,
                               /* event_wait_list */ NULL,
                               /* event */ NULL
                             );



    printf("\nReading back array:\n");
    printArray( copiedBackArray, arraySize);
    cleanUp();
    return 0;
}

void cleanUp()
{
    cl_int err=0;
    free(kernelSource);

    if (kernel!=0)
    {
        err = clReleaseKernel(kernel);
        handleError(err, "Couldn't release kernel", false);
    }

    if (program!= 0)
    {
        err = clReleaseProgram(program);
        handleError(err, "Couldn't release program", false);
    }

    if (cmdQueue != 0 )
    {
        err = clReleaseCommandQueue(cmdQueue);
        handleError(err, "Couldn't release command queue", false);
    }

    if (context!=0)
    {
        err = clReleaseContext(context);
        handleError(err, "Couldn't release context", false);
    }

    if (hostArrayA !=0)
        free(hostArrayA);

    if (hostArrayB !=0)
        free(hostArrayB);

    if (copiedBackArray!=0)
        free(copiedBackArray);
}
