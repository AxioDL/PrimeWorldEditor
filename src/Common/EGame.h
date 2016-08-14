#ifndef EGAME_H
#define EGAME_H

#include "TString.h"
#include "types.h"

class CFourCC;

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

#endif // EGAME_H
