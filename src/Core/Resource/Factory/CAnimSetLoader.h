#ifndef CCHARACTERLOADER_H
#define CCHARACTERLOADER_H

#include "Core/Resource/Animation/CAnimSet.h"
#include "Core/Resource/Animation/CSourceAnimData.h"
#include <Common/EGame.h>

class CAnimSetLoader
{
    TResPtr<CAnimSet> pSet;
    EGame mGame;

    CAnimSetLoader();
    CAnimSet* LoadCorruptionCHAR(IInputStream& rCHAR);
    CAnimSet* LoadReturnsCHAR(IInputStream& rCHAR);
    void LoadPASDatabase(IInputStream& rPAS4);
    void LoadParticleResourceData(IInputStream& rFile, SSetCharacter *pChar, uint16 Version);

    void LoadAnimationSet(IInputStream& rANCS);
    void ProcessPrimitives();

public:
    static CAnimSet* LoadANCS(IInputStream& rANCS, CResourceEntry *pEntry);
    static CAnimSet* LoadCHAR(IInputStream& rCHAR, CResourceEntry *pEntry);
    static CSourceAnimData* LoadSAND(IInputStream& rSAND, CResourceEntry *pEntry);
};

#endif // CCHARACTERLOADER_H
