#ifndef ENODETYPE_H
#define ENODETYPE_H

#include <Common/Flags.h>

enum class ENodeType : uint32
{
    None            = 0x0,
    Root            = 0x1,
    Model           = 0x2,
    Character       = 0x4,
    Static          = 0x8,
    Collision       = 0x10,
    Script          = 0x20,
    ScriptExtra     = 0x40,
    ScriptAttach    = 0x80,
    Light           = 0x100,
    All             = 0xFFFFFFFF
};

DECLARE_FLAGS_ENUMCLASS(ENodeType, FNodeFlags)

#endif // ENODETYPE_H

