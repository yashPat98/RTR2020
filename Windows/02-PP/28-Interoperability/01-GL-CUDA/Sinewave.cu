//headers
#include <cuda.h>

//cuda kernel definition
__global__ void sinewave_kernel(float4 *pos, unsigned int width, unsigned int height, float timer)
{   
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    float u, v, w;
    const float frequency = 4.0f;

    u = x / (float)width;
    v = y / (float)height;

    u = u * 2.0f - 1.0f;
    v = v * 2.0f - 1.0f;

    w = sinf(u * frequency + timer) * cosf(v * frequency + timer) * 0.5f;

    pos[y * width + x] = make_float4(u, w, v, 1.0f);
}

void launch_cuda_kernel(float4 *ppos, unsigned int width, unsigned int height, float timer)
{
    dim3 block = dim3(8, 8, 8);
    dim3 grid = dim3(width/block.x, height/block.y, 1);

    sinewave_kernel<<<grid, block>>>(ppos, width, height, timer);
}
