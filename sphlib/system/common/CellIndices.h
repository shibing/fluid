

#ifndef RTPS_CELLINDICES_H_INCLUDED
#define RTPS_CELLINDICES_H_INCLUDED


#include <RTPS.h>
#include <Buffer.h>

namespace rtps
{
    class CellIndices 
    {
        public:
            CellIndices() { cli = NULL; }
            CellIndices(const std::string& path, CL* cli);
            int execute(int num,
                    Buffer<unsigned int>& hashes,
                    Buffer<unsigned int>& ci_start,
                    Buffer<unsigned int>& ci_stop,
                    Buffer<GridParams>& gp,
                    int nb_cells,
                    Buffer<float4>& clf_debug,
                    Buffer<int4>& cli_debug);

        private:
            CL* cli;
            Kernel k_cellindices;
    };
}

#endif
