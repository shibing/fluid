

#ifndef RTPS_UTIL_H_INCLUDED
#define RTPS_UTIL_H_INCLUDED

#include "structs.h"
#include <vector>

#ifdef WIN32
    #if defined(rtps_EXPORTS)
        #define RTPS_EXPORT __declspec(dllexport)
    #else
        #define RTPS_EXPORT __declspec(dllimport)
	#endif 
#else
    #define RTPS_EXPORT
#endif

#include <opencl/Buffer.h>

namespace rtps
{

    char RTPS_EXPORT *file_contents(const char *filename, int *length);

    GLuint RTPS_EXPORT createVBO(const void* data, int dataSize, GLenum target, GLenum usage);
    int RTPS_EXPORT deleteVBO(GLuint id);


    void RTPS_EXPORT make_cube(std::vector<Triangle> &triangles, float4 center, float half_edge);


class Utils
{
public:
	void printDevArray(Buffer<unsigned int>& cl_array, char* msg, int nb_el, int nb_print);
	void printDevArray(Buffer<int>& cl_array, char* msg, int nb_el, int nb_print);
	void printDevArray(Buffer<int4>& cl_array, char* msg, int nb_el, int nb_print);
	void printDevArray(Buffer<float>& cl_array, char* msg, int nb_el, int nb_print);
	void printDevArray(Buffer<float4>& cl_array, char* msg, int nb_el, int nb_print);
};

}

#endif
