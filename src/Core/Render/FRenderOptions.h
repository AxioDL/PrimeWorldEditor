#ifndef FRENDEROPTIONS_H
#define FRENDEROPTIONS_H

#include <Common/Flags.h>

enum ERenderOption
{
    eNoRenderOptions     = 0x0,
    eDrawWorld           = 0x1,
    eDrawWorldCollision  = 0x2,
    eDrawObjects         = 0x4,
    eDrawObjectCollision = 0x8,
    eDrawLights          = 0x10,
    eDrawSky             = 0x20,
    eEnableUVScroll      = 0x40,
    eEnableBackfaceCull  = 0x80,
    eEnableOccluders     = 0x100,
    eNoMaterialSetup     = 0x200,
    eEnableBloom         = 0x400,
    eNoAlpha             = 0x800
};
DECLARE_FLAGS(ERenderOption, FRenderOptions)

#endif // FRENDEROPTIONS_H

