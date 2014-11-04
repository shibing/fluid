

#ifndef __NEIGHBORS_CL_K_
#define __NEIGHBORS_CL_K_

#include "cl_hash.h"

//----------------------------------------------------------------------
void zeroPoint(PointData* pt)
{
    pt->density = (float4)(0.,0.,0.,0.);
    pt->color = (float4)(0.,0.,0.,0.);
    pt->color_normal = (float4)(0.,0.,0.,0.);
    pt->force = (float4)(0.,0.,0.,0.);
    pt->surf_tens = (float4)(0.,0.,0.,0.);
    pt->color_lapl = 0.;
    pt->xsph = (float4)(0.,0.,0.,0.);
}

void IterateParticlesInCell(
                           ARGS,
                           PointData* pt,
                           uint num,
                           int4    cellPos,
                           uint    index_i,
                           float4  position_i,
                           __global int*       cell_indexes_start,
                           __global int*       cell_indexes_end,
                           __constant struct GridParams* gp,
                           __constant struct SPHParams* sphp
                           DEBUG_ARGS
                           )
{
    uint cellHash = calcGridHash(cellPos, gp->grid_res, false);
    
    if(cellHash >= gp->nb_cells) {
        return;
    }
    uint startIndex = FETCH(cell_indexes_start,cellHash);

    if (startIndex != 0xffffffff) {
        uint endIndex = FETCH(cell_indexes_end, cellHash);

        for (uint index_j = startIndex; index_j < endIndex; index_j++) {
            ForNeighbor(ARGV, pt, index_i, index_j, position_i, gp, sphp DEBUG_ARGV);
        }
    }
}

void IterateParticlesInNearbyCells(
                                  ARGS,
                                  PointData* pt,
                                  uint num,
                                  int     index_i, 
                                  float4   position_i, 
                                  __global int* cell_indices_start,
                                  __global int* cell_indices_end,
                                  __constant struct GridParams* gp,
                                  __constant struct SPHParams* sphp
                                  DEBUG_ARGS
                                  )
{
    int4 cell = calcGridCell(position_i, gp->grid_min, gp->grid_delta);

    for (int z = cell.z - 1; z <= cell.z + 1; ++z)
    {
        for (int y = cell.y - 1; y <= cell.y + 1; ++y)
        {
            for (int x = cell.x - 1; x <= cell.x + 1; ++x)
            {
                int4 ipos = (int4) (x,y,z,1);

                IterateParticlesInCell(ARGV, pt, num, ipos, index_i, position_i, cell_indices_start, cell_indices_end, gp, sphp DEBUG_ARGV);

            }
        }
    }
}

//----------------------------------------------------------------------

#endif
