#ifndef ENODETYPE_H
#define ENODETYPE_H

#include <Common/Flags.h>

enum ENodeType
{
    eRootNode           = 0x0,
    eModelNode          = 0x1,
    eCharacterNode      = 0x2,
    eStaticNode         = 0x4,
    eCollisionNode      = 0x8,
    eScriptNode         = 0x10,
    eScriptExtraNode    = 0x20,
    eScriptAttachNode   = 0x40,
    eLightNode          = 0x80,
    eAllNodeTypes       = 0xFFFFFFFF
};

DECLARE_FLAGS(ENodeType, FNodeFlags)

#endif // ENODETYPE_H

