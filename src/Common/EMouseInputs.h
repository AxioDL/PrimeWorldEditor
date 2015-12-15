#ifndef EMOUSEINPUTS
#define EMOUSEINPUTS

#include "EnumUtil.h"

enum EMouseInputs
{
    eLeftButton   = 0x1,
    eMiddleButton = 0x2,
    eRightButton  = 0x4
};
DEFINE_ENUM_FLAGS(EMouseInputs)

#endif // EMOUSEINPUTS

