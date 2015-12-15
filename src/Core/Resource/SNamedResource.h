#ifndef SNAMEDRESOURCE_H
#define SNAMEDRESOURCE_H

#include <Common/CFourCC.h>
#include <Common/types.h>

struct SNamedResource
{
    CFourCC resType;
    TString resName;
    u64 resID;
};

#endif // SNAMEDRESOURCE_H
