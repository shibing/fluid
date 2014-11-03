
#ifndef _CLHASH_CL_H_
#define _CLHASH_CL_H_



int4 calcGridCell(float4 p, float4 grid_min, float4 grid_delta)
{
    float4 pp;
    pp.x = (p.x-grid_min.x)*grid_delta.x;
    pp.y = (p.y-grid_min.y)*grid_delta.y;
    pp.z = (p.z-grid_min.z)*grid_delta.z;
    pp.w = (p.w-grid_min.w)*grid_delta.w;

    int4 ii;
    ii.x = (int) pp.x;
    ii.y = (int) pp.y;
    ii.z = (int) pp.z;
    ii.w = (int) pp.w;
    return ii;
}

//----------------------------------------------------------------------
int calcGridHash(int4 gridPos, float4 grid_res, bool wrapEdges
                 )
{
    int gx;
    int gy;
    int gz;

    if (wrapEdges)
    {
        int gsx = (int)floor(grid_res.x);
        int gsy = (int)floor(grid_res.y);
        int gsz = (int)floor(grid_res.z);

        gx = gridPos.x % gsx;
        gy = gridPos.y % gsy;
        gz = gridPos.z % gsz;
        if (gx < 0) gx+=gsx;
        if (gy < 0) gy+=gsy;
        if (gz < 0) gz+=gsz;
    }
    else
    {
        gx = gridPos.x;
        gy = gridPos.y;
        gz = gridPos.z;
    }

    //We choose to simply traverse the grid cells along the x, y, and z axes, in that order. The inverse of
    //this space filling curve is then simply:
    // index = x + y*width + z*width*height
    //This means that we process the grid structure in "depth slice" order, and
    //each such slice is processed in row-column order.


    // uint(-3) = 0   (so hash is never less than zero)
    // But if particle leaves boundary to the right of the grid, the hash
    // table can go out of bounds and the code might crash. This can happen
    // either if the boundary does not catch the particles or if the courant
    // condition is violated and the code goes unstable. 
    //  ^ this is resolved by checking hash

    return (gz*grid_res.y + gy) * grid_res.x + gx; 
}
#endif
