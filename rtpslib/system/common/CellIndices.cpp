#include <timer_eb.h>
#include <system/SPH.h>

#include <system/common/CellIndices.h>

#include <string>
#include <iostream>

namespace rtps
{

    CellIndices::CellIndices(std::string path, CL* cli_)
    {
        cli = cli_;
        path = path + "/cellindices.cl";
        k_cellindices = Kernel(cli, path, "cellindices");
        std::cout << "Load cell indices kernel" << std::endl;
    }

    int CellIndices::execute(int num,
                    Buffer<unsigned int>& hashes,
                    Buffer<unsigned int>& ci_start,
                    Buffer<unsigned int>& ci_end,
                    Buffer<GridParams>& gp,
                    int nb_cells,               
                    Buffer<float4>& clf_debug,
                    Buffer<int4>& cli_debug)
    {

        int minus = 0xffffffff;
        std::vector<unsigned int> ci_start_v(nb_cells + 1);
        std::fill(ci_start_v.begin(), ci_start_v.end(), minus);
        ci_start.copyToDevice(ci_start_v);

        int iarg = 0;
        k_cellindices.setArg(iarg++, num);
        k_cellindices.setArg(iarg++, hashes.getDevicePtr());
        k_cellindices.setArg(iarg++, ci_start.getDevicePtr());
        k_cellindices.setArg(iarg++, ci_end.getDevicePtr());
        k_cellindices.setArg(iarg++, gp.getDevicePtr());

        int workSize = 64;
        int nb_bytes = (workSize + 1) * sizeof(int);
        k_cellindices.setArgShared(iarg++, nb_bytes);
        
        try
        {
            float gputime = k_cellindices.execute(num, workSize);
        }
        catch (cl::Error er)
        {
            printf("ERROR(cellindices): %s(%s)\n", er.what(), oclErrorString(er.err()));
        }

        std::vector<unsigned int> num_changed(1);
        ci_start.copyToHost(num_changed, nb_cells);
       
        int nc = num_changed[0];
        return nc;
    }

}
