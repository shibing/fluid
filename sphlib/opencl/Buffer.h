
#ifndef RTPS_BUFFER_H_INCLUDED
#define RTPS_BUFFER_H_INCLUDED

#include <string>
#include <vector>

#include "CLL.h"
#ifdef WIN32
        #define RTPS_EXPORT __declspec(dllexport)
#else
    #define RTPS_EXPORT
#endif

namespace rtps
{
    template <class T>
    class RTPS_EXPORT Buffer
    {
    public:
        Buffer(){ cli=NULL; vbo_id=0; };
        //create an OpenCL buffer from existing data
        Buffer(CL *cli, const std::vector<T> &data);
        Buffer(CL *cli, const std::vector<T> &data, unsigned int memtype);
        //create a OpenCL BufferGL from a vbo_id
        Buffer(CL *cli, GLuint vbo_id);
        Buffer(CL *cli, GLuint vbo_id, int type);
        ~Buffer();

        cl_mem getDevicePtr() { return cl_buffer[0](); }
        cl::Memory& getBuffer(int index) {return cl_buffer[index];};
       
        //need to acquire and release arrays from OpenGL context if we have a VBO
        void acquire();
        void release();

        void copyToDevice(const std::vector<T> &data);
        void copyToDevice(const std::vector<T> &data, int start);

        std::vector<T> copyToHost(int num);
        std::vector<T> copyToHost(int num, int start);
        void copyToHost(std::vector<T> &data);
        void copyToHost(std::vector<T> &data, int start);

        void copyFromBuffer(Buffer<T> dst, size_t start_src, size_t start_dst, size_t size);
        
    private:
        std::vector<cl::Memory> cl_buffer;
		int nb_el; 
		int nb_bytes; 
        CL *cli;
        GLuint vbo_id;
    };
    #include "Buffer.cpp"
}
#endif

