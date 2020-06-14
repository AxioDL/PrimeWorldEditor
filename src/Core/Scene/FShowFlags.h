#ifndef FSHOWFLAGS
#define FSHOWFLAGS

#include <Common/Flags.h>

enum class EShowFlag : uint32
{
    None                = 0x00,
    SplitWorld          = 0x01,
    MergedWorld         = 0x02,
    WorldCollision      = 0x04,
    ObjectGeometry      = 0x08,
    ObjectCollision     = 0x10,
    Objects             = 0x18,
    Lights              = 0x20,
    Sky                 = 0x40,
    Skeletons           = 0x80,
    All                 = 0xFFFFFFFF
};
DECLARE_FLAGS_ENUMCLASS(EShowFlag, FShowFlags)

constexpr inline FShowFlags gkGameModeShowFlags{EShowFlag::MergedWorld | EShowFlag::ObjectGeometry | EShowFlag::Sky};

#endif // FSHOWFLAGS

