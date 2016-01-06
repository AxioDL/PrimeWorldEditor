#ifndef FRENDEROPTIONS_H
#define FRENDEROPTIONS_H

#include <Common/Flags.h>

enum ERenderOption
{
    eNoRenderOptions     = 0x0,
    eEnableUVScroll      = 0x1,
    eEnableBackfaceCull  = 0x2,
    eEnableOccluders     = 0x4,
    eNoMaterialSetup     = 0x8,
    eEnableBloom         = 0x10,
    eNoAlpha             = 0x20
};
DECLARE_FLAGS(ERenderOption, FRenderOptions)

#endif // FRENDEROPTIONS_H

