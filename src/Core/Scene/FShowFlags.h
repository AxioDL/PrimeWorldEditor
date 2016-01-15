#ifndef FSHOWFLAGS
#define FSHOWFLAGS

#include <Common/Flags.h>

enum EShowFlag
{
    eShowNone               = 0x00,
    eShowSplitWorld         = 0x01,
    eShowMergedWorld        = 0x02,
    eShowWorldCollision     = 0x04,
    eShowObjectGeometry     = 0x08,
    eShowObjectCollision    = 0x10,
    eShowObjects            = 0x18,
    eShowLights             = 0x20,
    eShowSky                = 0x40,
    eShowAll                = 0x7F
};
DECLARE_FLAGS(EShowFlag, FShowFlags)

extern const FShowFlags gkGameModeShowFlags;

#endif // FSHOWFLAGS

