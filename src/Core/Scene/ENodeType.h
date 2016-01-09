#ifndef ENODETYPE_H
#define ENODETYPE_H

#include <Common/Flags.h>

enum ENodeType
{
    eRootNode           = 0x0,
    eModelNode          = 0x1,
    eStaticNode         = 0x2,
    eCollisionNode      = 0x4,
    eScriptNode         = 0x8,
    eScriptExtraNode    = 0x10,
    eLightNode          = 0x20,
    eAllNodeTypes       = 0x3F
};

DECLARE_FLAGS(ENodeType, FNodeFlags)

#endif // ENODETYPE_H

