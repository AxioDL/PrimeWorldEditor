#ifndef EGAME_H
#define EGAME_H

#include "TString.h"
#include "types.h"

class CFourCC;
class IArchive;

// Note: A lot of code does inequality comparisons on EGame, ie. "if (Game <= eEchoes)", which means that the
// enum values need to be in chronological order. It'd be more convenient if the values were fourCCs, but we can't do that.
enum EGame
{
    ePrimeDemo,
    ePrime,
    eEchoesDemo,
    eEchoes,
    eCorruptionProto,
    eCorruption,
    eReturns,
    eUnknownGame = -1
};

CFourCC GetGameID(EGame Game);
EGame GetGameForID(const CFourCC& rkID);
TString GetGameName(EGame Game);
TString GetGameShortName(EGame Game);
void Serialize(IArchive& rArc, EGame& rGame);

// ERegion
enum class ERegion
{
    NTSC,
    PAL,
    JPN,
    Unknown = -1
};
const char* GetRegionName(ERegion Region);
ERegion GetRegionForName(const char* pkName);

#endif // EGAME_H
