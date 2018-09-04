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

TString GetGameName(EGame Game)
{
    switch (Game)
    {
    case ePrimeDemo:        return "Metroid Prime Kiosk Demo";
    case ePrime:            return "Metroid Prime";
    case eEchoesDemo:       return "Metroid Prime 2: Echoes Demo";
    case eEchoes:           return "Metroid Prime 2: Echoes";
    case eCorruptionProto:  return "Metroid Prime 3: Corruption Prototype";
    case eCorruption:       return "Metroid Prime 3: Corruption";
    case eReturns:          return "Donkey Kong Country Returns";
    default:                return "Unknown Game";
    }
}

TString GetGameShortName(EGame Game)
{
    switch (Game)
    {
    case ePrimeDemo:        return "MP1Demo";
    case ePrime:            return "MP1";
    case eEchoesDemo:       return "MP2Demo";
    case eEchoes:           return "MP2";
    case eCorruptionProto:  return "MP3Proto";
    case eCorruption:       return "MP3";
    case eReturns:          return "DKCR";
    default:                return "Unknown";
    }
}

void Serialize(IArchive& rArc, EGame& rGame)
{
    CFourCC GameID = GetGameID(rGame);
    rArc.SerializePrimitive(GameID, 0);
    if (rArc.IsReader()) rGame = GetGameForID(GameID);
}

// ERegion
static const TString gskRegionNames[] = { "NTSC", "PAL", "JPN", "UnknownRegion" };
static const u32 gskNumRegions = sizeof(gskRegionNames) / sizeof(gskRegionNames[0]);

TString GetRegionName(ERegion Region)
{
    return gskRegionNames[(int) Region];
}

ERegion GetRegionForName(const TString& rkName)
{
    for (u32 iReg = 0; iReg < gskNumRegions; iReg++)
        if (gskRegionNames[iReg] == rkName)
            return (ERegion) iReg;

    return eRegion_Unknown;
}
