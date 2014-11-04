
#ifndef RTPS_CLOUD_BITONIC_SORT_H
#define RTPS_CLOUD_BITONIC_SORT_H

#include "CLL.h"
#include "../opencl/Kernel.h"
#include "../opencl/Buffer.h"
//#include "../opencl/Scopy.h"

#ifndef uint
#define uint unsigned int
#endif

static const uint CLOUD_LOCAL_SIZE_LIMIT = 512U;

namespace rtps
{

template <class T>
class CloudBitonic
{
public:
    CloudBitonic(){ cli=NULL; };
    CloudBitonic( std::string source_dir, CL *cli );

    int Sort(int batch, int arrayLength, int dir,
                Buffer<T> *dstkey, Buffer<T> *dstval, 
                Buffer<T> *srckey, Buffer<T> *srcval);
    void loadKernels(std::string source_dir);



private:
    Kernel k_bitonicSortLocal, k_bitonicSortLocal1;
    Kernel k_bitonicMergeLocal, k_bitonicMergeGlobal;

    CL *cli;

};

#include "CloudBitonicSort.cpp"

}

#endif
