#ifndef EKEYINPUTS
#define EKEYINPUTS

#include "EnumUtil.h"

enum EKeyInputs
{
    eNoKeys  = 0,
    eCtrlKey = 0x1,
    eAltKey  = 0x2,
    eQKey    = 0x4,
    eWKey    = 0x8,
    eEKey    = 0x10,
    eAKey    = 0x20,
    eSKey    = 0x40,
    eDKey    = 0x80
};
DEFINE_ENUM_FLAGS(EKeyInputs)

#endif // EKEYINPUTS

