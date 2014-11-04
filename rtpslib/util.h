

#ifndef RTPS_UTIL_H_INCLUDED
#define RTPS_UTIL_H_INCLUDED

#include <structs.h>
#include <vector>
#include <rtps_common.h>

#include <opencl/Buffer.h>

namespace rtps
{
char RTPS_EXPORT *file_contents(const char *filename, int *length);

}

#endif
