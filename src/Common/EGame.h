#ifndef EGAME_H
#define EGAME_H

#include "TString.h"
#include "types.h"

class CFourCC;
class IArchive;

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
void Serialize(IArchive& rArc, EGame& rGame);

#endif // EGAME_H
