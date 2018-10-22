#ifndef EGAME_H
#define EGAME_H

#include "TString.h"
#include "types.h"

class CFourCC;
class IArchive;

//@todo I'm not really happy with EGame being in Common (would like Common to be more
// generic so it's more reusable between different projects) but atm can't think of
// any other decent way to integrate it with IArchive unfortunately
enum class EGame
{
    PrimeDemo,
    Prime,
    EchoesDemo,
    Echoes,
    CorruptionProto,
    Corruption,
    DKCReturns,
    
    Max,
    Invalid = -1
};

TString GetGameName(EGame Game);
TString GetGameShortName(EGame Game);
CFourCC GameTo4CC(EGame Game);
EGame GameFrom4CC(CFourCC GameId);
void Serialize(IArchive& rArc, EGame& rGame);

// ERegion
enum class ERegion
{
    NTSC,
    PAL,
    JPN,
    Unknown = -1
};

#endif // EGAME_H
