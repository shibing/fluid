

#ifndef _WPOLY6_CL_
#define _WPOLY6_CL_

//----------------------------------------------------------------------
float Wpoly6(float4 r, float h, __constant struct SPHParams* params)
{
    float r2 = r.x*r.x + r.y*r.y + r.z*r.z;  // dist_squared(r);

    float hr2 = (h*h-r2); 
    return hr2*hr2*hr2;

}
//----------------------------------------------------------------------
float Wpoly6_dr(float r, float h, __constant struct SPHParams* params)
{
    float x = (h * h - r * r);
    return x * x;
}
//----------------------------------------------------------------------
float Wpoly6_lapl(float4 r, float h, __constant struct SPHParams* params)
{
    // Laplacian
    float r2 = r.x*r.x + r.y*r.y + r.z*r.z;  // dist_squared(r);
    float h2 = h*h;
    float h3 = h2*h;
    float alpha = -945.f/(32.0f*params->PI*h3*h3*h3);
    float Wij = alpha*(h2-r2)*(2.*h2-7.f*r2);
    return Wij;
}
//----------------------------------------------------------------------
float Wspiky(float rlen, float h, __constant struct SPHParams* params)
{
    float h6 = h*h*h * h*h*h;
    float alpha = 15.f/params->PI/h6;
    float hr2 = (h - rlen);
    float Wij = alpha * hr2*hr2*hr2;
    return Wij;
}

//----------------------------------------------------------------------
float Wspiky_dr(float rlen, float h, __constant struct SPHParams* params)
{
    // Derivative with respect to |r| divided by |r|
    //   W_{|r|}/r = (45/pi*h^6) (h-|r|)^2 (-1) / r
    float hr2 = h - rlen;
    //return -hr2*hr2/rlen;
    //return hr2*hr2/(rlen + params->EPSILON);
    return hr2*hr2/rlen;
}

//----------------------------------------------------------------------
float Wvisc(float rlen, float h, __constant struct SPHParams* params)
{
    float alpha = 15./(2.*params->PI*h*h*h);
    float rh = rlen / h;
    float Wij = rh*rh*(1.-0.5*rh) + 0.5/rh - 1.;
    return alpha*Wij;
}
//----------------------------------------------------------------------
float Wvisc_dr(float rlen, float h, __constant struct SPHParams* params)
// Derivative with respect to |r| divided by |r|
// 
{
    float alpha = 15./(2.*params->PI * h*h*h);
    float rh = rlen / h;
    float Wij = (-1.5*rh + 2.)/(h*h) - 0.5/(rh*rlen*rlen);
    return Wij;
}
//----------------------------------------------------------------------
float Wvisc_lapl(float rlen, float h, __constant struct SPHParams* params)
{
    /*
    float h3 = h*h*h;
    float alpha = 45./(params->PI * h3*h3); 
    float Wij = alpha*(h-rlen);
    return Wij;
    */
    return h - rlen;
}

float Wspline(float r, float h, __constant struct SPHParams* params)
{
    if(r > h / 2.0f && r <= h)
        return (h - r) * (h - r) * (h - r) * r * r * r;
    else if(r > 0 && r < h / 2.0f)
        return 2 * (h - r) * (h - r) * (h - r) * r * r * r - (h * h * h * h * h * h) / 64.0f;
    return 0;
}

//----------------------------------------------------------------------

//_WPOLY6_CL_
#endif 
