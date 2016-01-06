#ifndef FSHOWFLAGS
#define FSHOWFLAGS

#include <Common/Flags.h>

enum EShowFlag
{
    eShowNone               = 0x00,
    eShowWorld              = 0x01,
    eShowWorldCollision     = 0x02,
    eShowObjectGeometry     = 0x04,
    eShowObjectCollision    = 0x08,
    eShowObjects            = 0x0C,
    eShowLights             = 0x10,
    eShowSky                = 0x20,
    eShowAll                = 0x3F
};
DECLARE_FLAGS(EShowFlag, FShowFlags)

extern const FShowFlags gkGameModeShowFlags;

#endif // FSHOWFLAGS

