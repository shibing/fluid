
#ifndef DOMAIN_H_INCLUDED
#define DOMAIN_H_INCLUDED

#include "../structs.h"
#include <vector>
#include "../rtps_common.h"

namespace rtps {



class RTPS_EXPORT Domain
{
public:
    Domain(){};
    
    Domain(float4 min, float4 max);

    ~Domain();

     void calculateCells(float cell_size);

     float4 getMin(){ return min; };

     float4 getMax(){ return max; };

     float4 getBndMin(){ return bnd_min; };

     void setBndMin(float4 new_bnd_min){ bnd_min = new_bnd_min; };
     
     float4 getBndMax(){ return bnd_max; };
     
     void setBndMax(float4 new_bnd_max){ bnd_max = new_bnd_max; };
     
     float4 getDelta() { return delta; };

     float4 getRes() { return res; };

     float4 getSize() { return size; };


private:
    float4 min;
    float4 max; 
	float4 bnd_min;
	float4 bnd_max;

    float4 size;
    float4 res;
    float4 delta;

};

struct GridParams
{
    float4          grid_size;
    float4          grid_min;
    float4          grid_max;

    float4          bnd_min; 
    float4          bnd_max;

    float4          grid_res;
    float4          grid_delta;
	int 			nb_cells; 

	void print()
	{
		printf("\n----- GridParams ----\n");
		grid_min.print("grid_min"); 
		grid_max.print("grid_max"); 
		bnd_min.print("bnd_min"); 
		bnd_max.print("bnd_max"); 
		grid_res.print("grid_res"); 
		grid_size.print("grid_size"); 
		grid_delta.print("grid_delta"); 
		printf("nb grid cells: %d\n", nb_cells);
	}
};





}
#endif
