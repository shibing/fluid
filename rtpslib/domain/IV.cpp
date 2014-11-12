
#include "IV.h"
#include <vector>

#include<stdlib.h>
#include<time.h>

namespace rtps
{

//----------------------------------------------------------------------
    std::vector<float4> addxyPlane(int num, float4 min, float4 max, float spacing, float scale, float zlevel, std::vector<float4>& normals)
    {
        spacing *= 1.1f;
        float xmin = min.x / scale;
        float xmax = max.x / scale;
        float ymin = min.y / scale;
        float ymax = max.y / scale;
        float zmin = zlevel / scale;
        float zmax = zlevel / scale;


        std::vector<float4> rvec(num);
		normals.resize(0);
        int i=0;

        for (float z = zmin; z <= zmax; z+=spacing)
        {
            for (float y = ymin; y <= ymax; y+=spacing)
            {
                for (float x = xmin; x <= xmax; x+=spacing)
                {
                    if (i >= num) break;
                    rvec[i] = float4(x,y,z,1.0f);

					float4& n = normals[i];

					n.x = 0.;
					n.y = 0.;
					n.z = 1.;
					n.w = 0.;
					normals.push_back(n);
					i++;
                }
            }
        }
        rvec.resize(i);
        return rvec;
    }
//----------------------------------------------------------------------
    std::vector<float4> addRect(int num, float4 min, float4 max, float spacing, float scale)
    {
        spacing *= 1.1f;

        float xmin = min.x / scale;
        float xmax = max.x / scale;
        float ymin = min.y / scale;
        float ymax = max.y / scale;
        float zmin = min.z / scale;
        float zmax = max.z / scale;

        std::vector<float4> rvec(num);
        int i=0;
        for (float z = zmin; z <= zmax; z+=spacing)
        {
            for (float y = ymin; y <= ymax; y+=spacing)
            {
                for (float x = xmin; x <= xmax; x+=spacing)
                {
                    if (i >= num) break;
                    rvec[i] = float4(x,y,z,1.0f);
                    i++;
                }
            }
        }
        rvec.resize(i);
        return rvec;

    }

    void addCube(int num, float4 min, float4 max, float spacing, float scale, std::vector<float4>& rvec)
    {
        float xmin = min.x / scale;
        float xmax = max.x / scale;
        float ymin = min.y / scale;
        float ymax = max.y / scale;
        float zmin = min.z / scale;
        float zmax = max.z / scale;


        rvec.resize(num);

        int i=0;
        for (float z = zmin; z <= zmax+.0*(zmax-zmin); z+=spacing) {
        for (float y = ymin; y <= ymax+.0*(ymax-ymin); y+=spacing) {
        for (float x = xmin; x <= xmax+.0*(xmax-xmin); x+=spacing) {
            if (i >= num) break;				
            rvec[i] = float4(x,y,z,1.0f);
            i++;
        }}
        }
        rvec.resize(i);

    }

	//----------------------------------------------------------------------
    std::vector<float4> addSphere(int num, float4 center, float radius, float spacing, float scale)
    {
        spacing *= 1.9f;
        float xmin = (center.x - radius) / scale;
        float xmax = (center.x + radius) / scale;
        float ymin = (center.y - radius) / scale;
        float ymax = (center.y + radius) / scale;
        float zmin = (center.z - radius) / scale;
        float zmax = (center.z + radius) / scale;
        float r2 = radius*radius;
        float d2 = 0.0f;

        std::vector<float4> rvec(num);
        int i=0;
        for (float z = zmin; z <= zmax; z+=spacing)
        {
            for (float y = ymin; y <= ymax; y+=spacing)
            {
                for (float x = xmin; x <= xmax; x+=spacing)
                {
                    if (i >= num) break;
                    d2 = (x - center.x)*(x - center.x) + (y - center.y)*(y - center.y) + (z - center.z)*(z - center.z);
                    if (d2 > r2) continue;
                    rvec[i] = float4(x,y,z,1.0f);
                    i++;
                }
            }
        }
        rvec.resize(i);
        return rvec;
    }
	//----------------------------------------------------------------------


    std::vector<float4> addDisc(int num, float4 center, float4 u, float4 v, float radius, float spacing)
    {

        spacing *= 1.999f; //should probably just figure out whats up with my spacing
        float4 umin = -radius*u;
        float4 vmin = -radius*v;

        std::vector<float4> rvec;
        int i = 0;
        float d2 = 0.;
        float r2 = radius*radius;
        for (float du = 0.; du < 2.*radius; du += spacing)
        {
            for (float dv = 0.; dv < 2.*radius; dv += spacing)
            {
                if (i >= num) break;
                float4 part = center + umin + u*du + vmin + v*dv;
                part.w = 1.0f;
                d2 = dist_squared(part-center);
                if (d2 < r2)
                {
                    rvec.push_back(part);
                    i++;
                }
            }
        }
        return rvec;

    }

    void addCylinder(const float4& center,  float radius, float spacing, std::vector<float4>& particles)
    {
        spacing *= 0.8f;
        for(float x = center.x; x <= center.x + 5 * spacing; x += spacing) {
            for(float y = center.y - radius; y <= center.y + radius; y += spacing) {
                for(float z = center.z - radius; z <= center.z + radius; z += spacing) {
                    float px = x;
                    float py = y; 
                    float pz = z;
                    if((py - center.y) * (py - center.y) + (pz - center.z) * (pz - center.z) > radius * radius)
                        continue;
                    particles.push_back(float4(px, py, pz, 1.0));
                }
            }
        }

    }

    std::vector<float4> addDiscRandom(int num, float4 center, float4 v, float4 u, float4 w, float radius, float spacing)
    {
        spacing *= 1.1f; //should probably just figure out whats up with my spacing
        float pert = .1f*spacing;   //amount of perterbation
        float vpert = 100.f;

        float4 umin = -radius*u;
        float4 wmin = -radius*w;

        std::vector<float4> rvec;
        int i = 0;
        float d2 = 0.;
        float r2 = radius*radius;
        for (float du = 0.; du < 2.*radius; du += spacing)
        {
            for (float dw = 0.; dw < 2.*radius; dw += spacing)
            {
                if (i >= num) break;
                float rrv = rand()*vpert*2./RAND_MAX - vpert;   //random number between -pert and pert
                float rru = rand()*pert*2./RAND_MAX - pert;   //random number between -pert and pert
                float rrw = rand()*pert*2./RAND_MAX - pert;   //random number between -pert and pert
                du += rru;
                dw += rrw;
                float4 part = center + umin + u*du + wmin + w*dw + rrv*v;
                part.w = 1.0f;
                d2 = dist_squared(part-center);
                if (d2 < r2) {
                    rvec.push_back(part);
                    i++;
                }
            }
        }
        return rvec;

    }



    std::vector<float4> addRandRect(int num, float4 min, float4 max, float spacing, float scale, float4 dmin, float4 dmax)
    {

        srand(time(NULL));	

        spacing *= 1.1f;
        float xmin = min.x  / scale;
        float xmax = max.x  / scale;
        float ymin = min.y  / scale;
        float ymax = max.y  / scale;
        float zmin = min.z  / scale;
        float zmax = max.z  / scale;

        std::vector<float4> rvec(num);
        int i=0;
        for (float z = zmin; z <= zmax; z+=spacing) {
        for (float y = ymin; y <= ymax; y+=spacing) {
        for (float x = xmin; x <= xmax; x+=spacing) {
            if (i >= num) break;	
            rvec[i] = float4(x-rand()/RAND_MAX,y-rand()/RAND_MAX,z-rand()/RAND_MAX,1.0f);

            i++;
        }}}
        rvec.resize(i);
        return rvec;
    }


    std::vector<float4> addRandSphere(int num, float4 center, float radius, float spacing, float scale, float4 dmin, float4 dmax)
    {
        srand(time(NULL));	
        
        spacing *= 1.1;

        float xmin = (center.x - radius)  / scale;
        float xmax = (center.x + radius)  / scale;
        float ymin = (center.y - radius)  / scale;
        float ymax = (center.y + radius)  / scale;
        float zmin = (center.z - radius)  / scale;
        float zmax = (center.z + radius)  / scale;
        
        float r2 = radius*radius;
        float d2 = 0.0f;

        std::vector<float4> rvec(num);
        int i=0;
        for (float z = zmin; z <= zmax; z+=spacing) {
        for (float y = ymin; y <= ymax; y+=spacing) {
        for (float x = xmin; x <= xmax; x+=spacing) {
            if (i >= num) break;				
            d2 = (x - center.x)*(x - center.x) + (y - center.y)*(y - center.y) + (z - center.z)*(z - center.z);
            if(d2 > r2) continue;
            rvec[i] = float4(x-rand()/RAND_MAX,y-rand()/RAND_MAX,z-rand()/RAND_MAX,1.0f);
            i++;
        }}}
        rvec.resize(i);
        return rvec;
    }

    std::vector<float4> addRandArrangement(int num, float scale, float4 dmin, float4 dmax)
    {

            srand(time(NULL));	
        
            std::vector<float4> rvec(num);
        int i=0;

        for(int z=dmin.z/scale; z <= dmax.z/scale; z+=0.3){
        for(int y=dmin.y/scale; y <= dmax.y/scale; y+=0.3){
        for(int x=dmin.x/scale; x <= dmax.x/scale; x+=0.3){
            if(i >= num) break;
            rvec[i] = float4(rand()/RAND_MAX,rand()/RAND_MAX,rand()/RAND_MAX,1.0f);
            i++;
        }}}
        rvec.resize(i);
        return rvec;
    }

//----------------------------------------------------------------------
	std::vector<float4> addHollowSphere(int num, float4 center, float radius_in, float radius_out, float spacing, float scale, std::vector<float4>& normals)
	{

        spacing *= 1.9f;
        float xmin = (center.x - radius_out) / scale;
        float xmax = (center.x + radius_out) / scale;
        float ymin = (center.y - radius_out) / scale;
        float ymax = (center.y + radius_out) / scale;
        float zmin = (center.z - radius_out) / scale;
        float zmax = (center.z + radius_out) / scale;
        float r2in  = (radius_in/scale)  * (radius_in/scale);
        float r2out = (radius_out/scale) * (radius_out/scale);
        float d2 = 0.0f;

		center = center /scale;

        std::vector<float4> rvec; // num
		normals.resize(0);
		int i=0;

        for (float z = zmin; z <= zmax; z+=spacing)
        {
            for (float y = ymin; y <= ymax; y+=spacing)
            {
                for (float x = xmin; x <= xmax; x+=spacing)
                {
                    if (i >= num) break;

					if (x < 0 || x >5) continue;
					if (y < 0 || y >5) continue;
					if (z < 0 || z >5) continue;

					float4 n(x-center.x, y-center.y, z-center.z, 0.);
                    d2 = (x - center.x)*(x - center.x) + (y - center.y)*(y - center.y) + (z - center.z)*(z - center.z);

                    if (d2 > r2out || d2 < r2in) continue;
					float4 r(x,y,z,1.0f);
					float sqi = sqrt(1. / (n.x*n.x + n.y*n.y + n.z*n.z));
					n.x *= sqi;
					n.y *= sqi;
					n.z *= sqi;
					n.w  = 0.;
					rvec.push_back(r);
					normals.push_back(n);
					i++;
                }
            }
        }
        return rvec;
	}
//----------------------------------------------------------------------



}

