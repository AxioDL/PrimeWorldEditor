#ifndef EMOUSEINPUTS
#define EMOUSEINPUTS

#include "Flags.h"

enum EMouseInput
{
    eLeftButton   = 0x1,
    eMiddleButton = 0x2,
    eRightButton  = 0x4
};
DECLARE_FLAGS(EMouseInput, FMouseInputs)

#endif // EMOUSEINPUTS

