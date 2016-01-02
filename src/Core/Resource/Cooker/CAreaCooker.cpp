#include "CAreaCooker.h"

CAreaCooker::CAreaCooker()
{
}

u32 CAreaCooker::GetMREAVersion(EGame version)
{
    switch (version)
    {
    case ePrimeDemo:       return 0xC;
    case ePrime:           return 0xF;
    case eEchoesDemo:      return 0x15;
    case eEchoes:          return 0x19;
    case eCorruptionProto: return 0x1D;
    case eCorruption:      return 0x1E;
    case eReturns:         return 0x20;
    default:               return 0;
    }
}
