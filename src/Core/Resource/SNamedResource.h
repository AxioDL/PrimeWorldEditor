#ifndef SNAMEDRESOURCE_H
#define SNAMEDRESOURCE_H

#include <Common/CFourCC.h>
#include <Common/types.h>

struct SNamedResource
{
    CFourCC Type;
    TString Name;
    u64 ID;
};

#endif // SNAMEDRESOURCE_H
