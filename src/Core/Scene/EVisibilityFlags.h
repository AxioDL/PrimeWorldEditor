#ifndef EVISIBILITYFLAGS
#define EVISIBILITYFLAGS

#include <Common/EnumUtil.h>

enum EVisibilityFlags
{
    eShowNomr               = 0x00,
    eShowWorld              = 0x01,
    eShowWorldCollision     = 0x02,
    eShowObjects            = 0x04,
    eShowObjectCollision    = 0x08,
    eShowLights             = 0x10,
    eShowAll                = 0x1F
};
DEFINE_ENUM_FLAGS(EVisibilityFlags)

#endif // EVISIBILITYFLAGS

