#ifndef FRENDEROPTIONS_H
#define FRENDEROPTIONS_H

#include <Common/Flags.h>

enum class ERenderOption
{
    None                = 0x0,
    EnableUVScroll      = 0x1,
    EnableBackfaceCull  = 0x2,
    EnableOccluders     = 0x4,
    NoMaterialSetup     = 0x8,
    EnableBloom         = 0x10,
    NoAlpha             = 0x20
};
DECLARE_FLAGS_ENUMCLASS(ERenderOption, FRenderOptions)

#endif // FRENDEROPTIONS_H

