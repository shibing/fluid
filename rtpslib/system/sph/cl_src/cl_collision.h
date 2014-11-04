

#ifndef _CL_COLLISION_H_
#define _CL_COLLISION_H_


float4 calculateRepulsionForce(float4 normal, float4 vel, float boundary_stiffness, float boundary_dampening, float distance)
{
    vel.w = 0.0f;
    float4 repulsion_force = (boundary_stiffness * distance - boundary_dampening * dot(normal, vel))*normal;
    repulsion_force.w = 0.0f;
    return repulsion_force;
}

float4 calculateFrictionForce(float4 vel, float4 force, float4 normal, float friction_kinetic, float friction_static_limit)
{
	float4 friction_force = (float4)(0.0f,0.0f,0.0f,0.0f);
    force.w = 0.0f;
    vel.w = 0.0f;

	float4 f_n = force * dot(normal, force);
	float4 f_t = force - f_n;

	float4 v_n = vel * dot(normal, vel);
	float4 v_t = vel - v_n;

	if((v_t.x + v_t.y + v_t.z)/3.0f > friction_static_limit)
		friction_force = -v_t;
	else
		friction_force = friction_kinetic * -v_t;
	
	return friction_force;

}

#endif
//----------------------------------------------------------------------
