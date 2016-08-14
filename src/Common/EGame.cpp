#include "EGame.h"
#include "CFourCC.h"

CFourCC GetGameID(EGame Game)
{
    switch (Game)
    {
    case ePrimeDemo:        return "MP1D";
    case ePrime:            return "MP1 ";
    case eEchoesDemo:       return "MP2D";
    case eEchoes:           return "MP2E";
    case eCorruptionProto:  return "MP3P";
    case eCorruption:       return "MP3C";
    case eReturns:          return "DKCR";
    default:                return "UNKN";
    }
}

EGame GetGameForID(const CFourCC& rkID)
{
    if (rkID == "MP1D") return ePrimeDemo;
    if (rkID == "MP1 ") return ePrime;
    if (rkID == "MP2D") return eEchoesDemo;
    if (rkID == "MP2E") return eEchoes;
    if (rkID == "MP3P") return eCorruptionProto;
    if (rkID == "MP3C") return eCorruption;
    if (rkID == "DKCR") return eReturns;
    return eUnknownGame;
}
