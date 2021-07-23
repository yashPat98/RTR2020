__kernel void sinewave_kernel(__global float4 *position, unsigned int width, unsigned int height, float timer)
{
    //variable declaration
    int x = get_global_id(0);
    int y = get_global_id(1);
    
    float u, v, w;
    const float frequency = 4.0f;

    u = x / (float)width;
    v = y / (float)height;

    u = u * 2.0f - 1.0f;
    v = v * 2.0f - 1.0f;

    w = sin(u * frequency + timer) * cos(v * frequency + timer) * 0.5f;

    position[y * width + x] = (float4)(u, w, v, 1.0f);
}

