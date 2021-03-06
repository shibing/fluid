

#ifndef HOSE_H_INCLUDED
#define HOSE_H_INCLUDED

#include <RTPS.h>
#include <vector>

#include <rtps_common.h>
using namespace std;

namespace rtps
{

    
class RTPS_EXPORT Hose
{
public:
    Hose(RTPS *ps, int total_n, float4 center, float4 velocity, float radius, float spacing, float4 color);

    void update(float4 center, float4 velocity, float radius, float spacing, float4 color);
    void refill(int num);
    std::vector<float4> spray();
    float4 getVelocity(){ return velocity;}
    float4 getColor(){ return color;}


private:
    int total_n;        //total particles available to the hose
    int n_count;        //number of particles left in the hose

    float4 center;
    float4 velocity;
    float4 color;
    float4 u, w;        //orthogonal vectors to velocity
    void calc_vectors();

    float radius;
    float spacing;

    void calc_em();     //calculate emission rate
    int em;             //how many calls to spray before emitting
    int em_count;       //what call we are on

    //we need the timestep and spacing from the settings
    RTPS *ps;

};

}

#endif
