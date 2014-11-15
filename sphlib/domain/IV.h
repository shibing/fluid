
#ifndef IV_H_INCLUDE
#define IV_H_INCLUDE

#include <vector>
#include <structs.h>
#include <vector>
#include <rtps_common.h>
using namespace std;

namespace rtps
{

    RTPS_EXPORT vector<float4> addRect(int num, float4 min, float4 max, float spacing, float scale);
    RTPS_EXPORT vector<float4> addSphere(int num, float4 center, float radius, float spacing, float scale);
    RTPS_EXPORT std::vector<float4> addDisc(int num, float4 center, float4 u, float4 v, float radius, float spacing);
    RTPS_EXPORT std::vector<float4> addDiscRandom(int num, float4 center, float4 vel, float4 u, float4 v, float radius, float spacing);
    RTPS_EXPORT void addCylinder(const float4& center, float radius, float spacing, std::vector<float4>& particles);

    RTPS_EXPORT void addCube(int num, float4 min, float4 max, float spacing, float scale, std::vector<float4>& rvec);
	RTPS_EXPORT void addBunny(const float4& center, std::vector<float4>& rvec);

    RTPS_EXPORT vector<float4> addRandRect(int num, float4 min, float4 max, float spacing, float scale, float4 dmin, float4 dmax);
    RTPS_EXPORT vector<float4> addRandArrangement(int num, float scale, float4 dmin, float4 dmax);
    RTPS_EXPORT vector<float4> addRandSphere(int num, float4 center, float radius, float spacing, float scale, float4 dmin, float4 dmax);
	RTPS_EXPORT std::vector<float4> addHollowSphere(int nn, float4 center, float radius_in, float radius_out, float spacing, float scale, std::vector<float4>& normals);
    RTPS_EXPORT std::vector<float4> addxyPlane(int num, float4 min, float4 max, float spacing, float scale, float zlevel, std::vector<float4>& normals);
}

#endif
