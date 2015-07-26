#ifndef ERENDEROPTIONS
#define ERENDEROPTIONS

#include <Common/EnumUtil.h>

enum ERenderOptions
{
    eEnableUVScroll     = 0x1,
    eEnableBackfaceCull = 0x2,
    eEnableOccluders    = 0x4,
    eNoMaterialSetup    = 0x8,
    eEnableBloom        = 0x10,
    eNoAlpha            = 0x20
};
DEFINE_ENUM_FLAGS(ERenderOptions)

#endif // ERENDEROPTIONS

