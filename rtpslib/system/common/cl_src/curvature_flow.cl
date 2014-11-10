#pragma OPENCL EXTENSION cl_amd_printf : enable

__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | 
    CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;


float mean_curvature(read_only image2d_t depth_in, int2 coords, const int width, const int height, const float fx, const float fy)
{
    //first compute normal
    float z        = read_imagef(depth_in, sampler, coords).x;
    float z_left   = read_imagef(depth_in, sampler, coords + (int2)(-1, 0)).x;
    float z_right  = read_imagef(depth_in, sampler, coords + (int2)(1, 0)).x;
    float z_down   = read_imagef(depth_in, sampler, coords + (int2)(0, -1)).x;
    float z_up     = read_imagef(depth_in, sampler, coords + (int2)(0, 1)).x;

    if(z_left == 0)
        z_left = z;
    if(z_right == 0)
        z_right = z;
    if(z_down == 0)
        z_down = z;
    if(z_up == 0)
        z_up = z;

    float z_x = (z_right - z_left) / 2;
    float z_y = (z_up - z_down) / 2;

    //four corner
    float z00 = read_imagef(depth_in, sampler, coords + (int2)(-1, -1)).x;
    float z10 = read_imagef(depth_in, sampler, coords + (int2)( 1, -1)).x;
    float z11 = read_imagef(depth_in, sampler, coords + (int2)( 1,  1)).x;
    float z01 = read_imagef(depth_in, sampler, coords + (int2)(-1,  1)).x;
    
    if(z00 == 0)
        z00 = z;
    if(z11 == 0)
        z11 = z;
    if(z10 == 0)
        z10 = z;
    if(z01 == 0)
        z01 = z;

    float z_xy = (z11 + z00 - z10 - z01) / 4;

    float z_xx = (z_right + z_left  - 2 * z);
    float z_yy = (z_up + z_down - 2 * z);

    const float Cx = -2.0/(width * fx);
    const float Cy = -2.0/(height * fy);

    float Wx = (width - 2 * coords.x) / (width * fx);
    float Wy = (height - 2 * coords.y) / (height * fy);

    float D = Cy * Cy * z_x * z_x + Cx * Cx * z_y * z_y + Cx * Cx * Cy * Cy * z * z;
    float D_x = 2 * Cy * Cy * z_x * z_xx + 2 * Cx * Cx * z_y * z_xy + 2 * Cx * Cx * Cy * Cy * z * z_x;
    float D_y = 2 * Cy * Cy * z_x * z_xy + 2 * Cx * Cx * z_y * z_yy + 2 * Cx * Cx * Cy * Cy * z * z_y;

    float Ex = 0.5 * z_x * D_x - z_xx * D;
    float Ey = 0.5 * z_y * D_y - z_yy * D;

    float H = (Cy * Ex + Cx * Ey)/(D * sqrt(D)) * 0.5;

    return H;
}

__kernel void curvature_flow(read_only image2d_t depth_in, write_only image2d_t depth_out, const float dt, const float z_contrib, const int width, const int height, const float fx, const float fy)
{
    int x = get_global_id(0);
    int y = get_global_id(1);
    int2 coords = (int2)(x, y);

    float depth = read_imagef(depth_in, sampler, coords).x;
    if(depth == 0) {
        write_imagef(depth_out, coords, (float4)(0, 0, 0, 1));
        return;
    }
    float H = mean_curvature(depth_in, coords, width, height, fx, fy);
    
    float depth_dt = depth + dt * H;
    write_imagef(depth_out, coords, (float4)(depth_dt, 0, 0, 1));
}
