
template <class T>
Bitonic<T>::Bitonic(std::string source_dir, CL *cli )
{
    this->cli = cli;
   loadKernels(source_dir);
}

template <class T>
void Bitonic<T>::loadKernels(std::string source_dir)
{
    std::string options = "-D LOCAL_SIZE_LIMIT=512";
    cl::Program prog = cli->loadProgram(source_dir + "/bitonic.cl", options);
    k_bitonicSortLocal = Kernel(cli, prog, "bitonicSortLocal");
    k_bitonicSortLocal1 = Kernel(cli, prog, "bitonicSortLocal1");
    k_bitonicMergeLocal = Kernel(cli, prog, "bitonicMergeLocal");
    k_bitonicMergeGlobal = Kernel(cli, prog, "bitonicMergeGlobal");
}

static cl_uint factorRadix2(cl_uint& log2L, cl_uint L){
    if(!L){
        log2L = 0;
        return 0;
    }else{
        for(log2L = 0; (L & 1) == 0; L >>= 1, log2L++);
        return L;
    }
}


template <class T>
int Bitonic<T>::Sort(int batch, int arrayLength, int dir,
                    Buffer<T> *cl_dstkey, Buffer<T> *cl_dstval, 
                    Buffer<T> *cl_srckey, Buffer<T> *cl_srcval)
{

    if(arrayLength < 2)
        return 0;

    int arg = 0;
    k_bitonicSortLocal.setArg(arg++, cl_dstkey->getDevicePtr());
    k_bitonicSortLocal.setArg(arg++, cl_dstval->getDevicePtr());
    k_bitonicSortLocal.setArg(arg++, cl_srckey->getDevicePtr());
    k_bitonicSortLocal.setArg(arg++, cl_srcval->getDevicePtr());

    arg = 0;
    k_bitonicSortLocal1.setArg(arg++, cl_dstkey->getDevicePtr());
    k_bitonicSortLocal1.setArg(arg++, cl_dstval->getDevicePtr());
    k_bitonicSortLocal1.setArg(arg++, cl_srckey->getDevicePtr());
    k_bitonicSortLocal1.setArg(arg++, cl_srcval->getDevicePtr());

    arg = 0;
    k_bitonicMergeGlobal.setArg(arg++, cl_dstkey->getDevicePtr());
    k_bitonicMergeGlobal.setArg(arg++, cl_dstval->getDevicePtr());
    k_bitonicMergeGlobal.setArg(arg++, cl_dstkey->getDevicePtr());
    k_bitonicMergeGlobal.setArg(arg++, cl_dstval->getDevicePtr());

    arg = 0;
    k_bitonicMergeLocal.setArg(arg++, cl_dstkey->getDevicePtr());
    k_bitonicMergeLocal.setArg(arg++, cl_dstval->getDevicePtr());
    k_bitonicMergeLocal.setArg(arg++, cl_dstkey->getDevicePtr());
    k_bitonicMergeLocal.setArg(arg++, cl_dstval->getDevicePtr());

    cl_uint log2L;
    cl_uint factorizationRemainder = factorRadix2(log2L, arrayLength);
    
    dir = (dir != 0);

    int localWorkSize;
    int globalWorkSize;

    if(arrayLength <= LOCAL_SIZE_LIMIT)
    {
         //Launch bitonicSortLocal
        k_bitonicSortLocal.setArg(4, arrayLength);
        k_bitonicSortLocal.setArg(5, dir); 

        localWorkSize  = LOCAL_SIZE_LIMIT / 2;
        globalWorkSize = batch * arrayLength / 2;
        k_bitonicSortLocal.execute(globalWorkSize, localWorkSize);
  
    }
    else
    {
        
        localWorkSize  = LOCAL_SIZE_LIMIT / 2;
        globalWorkSize = batch * arrayLength / 2;
        k_bitonicSortLocal1.execute(globalWorkSize, localWorkSize);

        for(uint size = 2 * LOCAL_SIZE_LIMIT; size <= arrayLength; size <<= 1)
        {
            for(unsigned stride = size / 2; stride > 0; stride >>= 1)
            {
                if(stride >= LOCAL_SIZE_LIMIT)
                {
                    //Launch bitonicMergeGlobal
                    k_bitonicMergeGlobal.setArg(4, arrayLength);
                    k_bitonicMergeGlobal.setArg(5, size);
                    k_bitonicMergeGlobal.setArg(6, stride);
                    k_bitonicMergeGlobal.setArg(7, dir); 

                    globalWorkSize = batch * arrayLength / 2;
                    k_bitonicMergeGlobal.execute(globalWorkSize);
                }
                else
                {
                    //Launch bitonicMergeLocal
                    
                    k_bitonicMergeLocal.setArg(4, arrayLength);
                    k_bitonicMergeLocal.setArg(5, stride);
                    k_bitonicMergeLocal.setArg(6, size);
                    k_bitonicMergeLocal.setArg(7, dir); 

                    localWorkSize  = LOCAL_SIZE_LIMIT / 2;
                    globalWorkSize = batch * arrayLength / 2;
                    
                    k_bitonicMergeLocal.execute(globalWorkSize, localWorkSize);
                    break;
                }
                //printf("globalWorkSize: %d\n", globalWorkSize);
            }
        }
    }

    return localWorkSize;
}
