#ifndef CCHARACTERLOADER_H
#define CCHARACTERLOADER_H

#include "Core/Resource/Animation/CAnimSet.h"
#include <Common/EGame.h>

class CAnimSetLoader
{
    TResPtr<CAnimSet> pSet;
    EGame mVersion;

    CAnimSetLoader();
    CAnimSet* LoadCorruptionCHAR(IInputStream& rCHAR);
    CAnimSet* LoadReturnsCHAR(IInputStream& rCHAR);
    void LoadPASDatabase(IInputStream& rPAS4, SSetCharacter *pChar);

    void LoadAnimationSet(IInputStream& rANCS);
    void ProcessPrimitives();

public:
    static CAnimSet* LoadANCSOrCHAR(IInputStream& rFile, CResourceEntry *pEntry);
    static CAnimSet* LoadANCS(IInputStream& rANCS, CResourceEntry *pEntry);
    static CAnimSet* LoadCHAR(IInputStream& rCHAR, CResourceEntry *pEntry);
};

#endif // CCHARACTERLOADER_H
