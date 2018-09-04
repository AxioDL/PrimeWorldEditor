#ifndef EGAME_H
#define EGAME_H

#include "TString.h"
#include "types.h"

class CFourCC;
class IArchive;

// Note: The reason why the EGame value isn't just the fourCC game ID is because a lot of code does inequality
// comparisons on EGame for version checking ie. "if (Game <= eEchoes)", which means the enum values need to be
// in chronological order.
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
enum ERegion
{
    eRegion_NTSC,
    eRegion_PAL,
    eRegion_JPN,
    eRegion_Unknown = -1
};
TString GetRegionName(ERegion Region);
ERegion GetRegionForName(const TString& rkName);

#endif // EGAME_H
