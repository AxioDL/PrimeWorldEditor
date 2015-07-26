#include "CTimer.h"
#include <cmath>

namespace AnimUtil
{
    float SecondsMod900()
    {
        return fmod((float) CTimer::GlobalTime(), 900.f);
    }
}
