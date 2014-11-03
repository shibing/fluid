
#ifndef _HASH_CL_H_
#define _HASH_CL_H_

#include "cl_structs.h"
#include "cl_macros.h"
#include "cl_hash.h"

__kernel void hash(
                  int num,
                  __global float4* pos_u,
                  __global uint* sort_hashes,
                  __global uint* sort_indexes,
                  __constant struct GridParams* gp
                  DEBUG_ARGS
                  )
{
    uint index = get_global_id(0);
    if (index >= num) return;

    float4 p = pos_u[index]; 

    int4 gridPos = calcGridCell(p, gp->grid_min, gp->grid_delta);
    bool wrap_edges = false;
    int hash = calcGridHash(gridPos, gp->grid_res, wrap_edges);

    cli[index].xyz = gridPos.xyz;
    cli[index].w = hash;

    hash = hash > gp->nb_cells ? gp->nb_cells : hash;
    hash = hash < 0 ? gp->nb_cells : hash;
   
    sort_hashes[index] = (uint)hash;

    sort_indexes[index] = index;
}


#endif
