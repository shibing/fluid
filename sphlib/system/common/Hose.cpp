
#include <render/Render.h>
#include <RTPSettings.h>
#include <RTPS.h>
#include <CLL.h>
#include <system/SPH.h>

#include <Hose.h>


namespace rtps
{

Hose::Hose(RTPS *ps, int total_n, float4 center, float4 velocity, float radius, float spacing, float4 color)
{
    this->ps = ps;
    this->total_n = total_n;
    this->center = center;
    this->velocity = velocity;
    this->radius = radius;
    this->spacing = spacing;
    this->color = color;
    em_count = 0;
    n_count = total_n;
    calc_vectors();
}

void Hose::update(float4 center, float4 velocity, float radius, float spacing, float4 color)
{
    this->center = center;
    this->velocity = velocity;
    this->radius = radius;
    this->spacing = spacing;
    this->color = color;
    calc_vectors();
}

void Hose::refill(int num)
{
    total_n = num;
    n_count = num;
}

void Hose::calc_vectors()
{
    if (velocity.y == 0.) velocity.y = .0000001;
    if (velocity.z == 0.) velocity.z = .0000001;
    u = float4(1., -(velocity.x/velocity.y), 0., 1.);
    float b = 1.;
    float a = b*velocity.x/velocity.y;
    float c = -b*(velocity.x*velocity.x + velocity.y*velocity.y)/(velocity.y*velocity.z);
    w = float4(a, b, c, 1.);
    u = normalize(u);
    w = normalize(w);
}

void Hose::calc_em()
{
    float dt = ps->settings->dt;
    float magv = magnitude(velocity);
    em = (int) (1 + spacing/dt/magv/10);
}

std::vector<float4> Hose::spray()
{
    em_count++;
    calc_em();
    std::vector<float4> particles;
    if(em_count >= em && n_count > 0) {
        float4 v = velocity * ps->settings->dt;
        particles = addDiscRandom(n_count, center, v, u, w, radius, spacing);
        n_count -= particles.size();
        em_count = 0;
    }
    return particles;
}
}
