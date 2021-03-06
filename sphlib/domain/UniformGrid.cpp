

#include <math.h>

#include "UniformGrid.h"

namespace rtps
{

    UniformGrid::UniformGrid(float4 min, float4 max, float cell_size)
    {
        this->min = min;
        this->max = max;
        size = float4(max.x - min.x,
                      max.y - min.y,
                      max.z - min.z,
                      0.0f);

        res = float4(ceil(size.x / cell_size),
                     ceil(size.y / cell_size),
                     ceil(size.z / cell_size),
                     0.0f);

        size = float4(res.x * cell_size,
                      res.y * cell_size,
                      res.z * cell_size,
                      0.0f);

        delta = float4(res.x / size.x,
                       res.y / size.y,
                       res.z / size.z,
                       0.0f);

    }

    UniformGrid::~UniformGrid()
    {
    }

    void UniformGrid::make_cube(float4* position, float spacing, int num)
    {
        //float xmin = min.x/2.5f;
        float xmin = min.x + max.x/2.5f;
        float xmax = max.x/4.0f;
        //float ymin = min.y;
        float ymin = min.y + max.y/4.0f;
        float ymax = max.y;
        //float zmin = min.z/2.0f;
        float zmin = min.z;// + max.z/2.0f;
        float zmax = min.z + max.z/2.0f;

        int i=0;
        //cube in corner
        for (float y = ymin; y <= ymax; y+=spacing)
        {
            for (float z = zmin; z <= zmax; z+=spacing)
            {
                for (float x = xmin; x <= xmax; x+=spacing)
                {
                    if (i >= num) break;
                    position[i] = float4(x,y,z,1.0f);
                    i++;
                }
            }
        }

    }


    void UniformGrid::make_column(float4* position, float spacing, int num)
    {
        //float xmin = min.x/2.5f;
        float xmin = min.x;
        float xmax = min.x + max.x;
        //float ymin = min.y;
        float ymin = min.y;
        float ymax = min.y + max.y;
        //float zmin = min.z/2.0f;
        float zmin = min.z;// + max.z/2.0f;
        float zmax = min.z + max.z;

        int i=0;
        for (float z = zmin; z <= zmax; z+=spacing)
        {
            for (float y = ymin; y <= ymax; y+=spacing)
            {
                for (float x = xmin; x <= xmax; x+=spacing)
                {
                    if (i >= num) break;
                    position[i] = float4(x,y,z,1.0f);
                    i++;
                }
            }
        }

    }
    void UniformGrid::make_dam(float4* position, float spacing, int num)
    {
        //float xmin = min.x/2.5f;
        float xmin = min.x;
        float xmax = min.x + max.x/2.0f;
        //float ymin = min.y;
        float ymin = min.y;
        float ymax = min.y + max.y;
        //float zmin = min.z/2.0f;
        float zmin = min.z;// + max.z/2.0f;
        float zmax = min.z + max.z;

        int i=0;
        for (float x = xmin; x <= xmax; x+=spacing)
        {
            for (float z = zmin; z <= zmax; z+=spacing)
            {
                for (float y = ymin; y <= ymax; y+=spacing)
                {
                    if (i >= num) break;
                    position[i] = float4(x,y,z,1.0f);
                    i++;
                }
            }
        }

    }

    int UniformGrid::make_line(float4* position, float spacing, int num)
    {
        float xmin = min.x;
        float xmax = max.x;
        int i = 0;
        float y = min.y + (max.y - min.y)/2.0f;
        float z = min.z + (max.z - min.z)/2.0f;
        for (float x = xmin; x <= xmax; x+= spacing)
        {
            if (i >= num) break;
            position[i] = float4(x,y,z,1.0f);
            i++;
        }
        return i;
    }

}
