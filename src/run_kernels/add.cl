__kernel void simple_kernel( __global int* A)
{
    int tid = get_global_id(0);
    A[tid]+=1;
}
