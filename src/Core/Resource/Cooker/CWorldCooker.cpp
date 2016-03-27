#include "CWorldCooker.h"

CWorldCooker::CWorldCooker()
{
}

u32 CWorldCooker::GetMLVLVersion(EGame Version)
{
    switch (Version)
    {
    case ePrimeDemo:  return 0xD;
    case ePrime:      return 0x11;
    case eEchoesDemo: return 0x14;
    case eEchoes:     return 0x17;
    case eCorruption: return 0x19;
    case eReturns:    return 0x1B;
    default:          return 0;
    }
}
