// This computes that dot product of an array
// with itself.
__kernel void simple_kernel( __global int* A)
{
    int tid = get_global_id(0);
    int n = get_global_size(0);
    A[tid] = A[tid]*A[tid];
    
    
    for (int d=n/2; n > 0 ; n /= 2)
    {
        if ( tid < d)
            A[tid] += A[tid + d];

        barrier(CLK_GLOBAL_MEM_FENCE);
    }

    // A[0] contains result
}
