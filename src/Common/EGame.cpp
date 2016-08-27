#include "EGame.h"
#include "CFourCC.h"
#include "Common/Serialization/IArchive.h"

CFourCC GetGameID(EGame Game)
{
    switch (Game)
    {
    case ePrimeDemo:        return "MP1D";
    case ePrime:            return "MPRM";
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
    if (rkID == "MPRM") return ePrime;
    if (rkID == "MP2D") return eEchoesDemo;
    if (rkID == "MP2E") return eEchoes;
    if (rkID == "MP3P") return eCorruptionProto;
    if (rkID == "MP3C") return eCorruption;
    if (rkID == "DKCR") return eReturns;
    return eUnknownGame;
}

void Serialize(IArchive& rArc, EGame& rGame)
{
    CFourCC GameID = GetGameID(rGame);
    rArc.SerializePrimitive(GameID);
    if (rArc.IsReader()) rGame = GetGameForID(GameID);
}
