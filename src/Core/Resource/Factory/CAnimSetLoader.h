#ifndef CCHARACTERLOADER_H
#define CCHARACTERLOADER_H

#include "Core/Resource/Animation/CAnimSet.h"
#include "Core/Resource/Animation/CSourceAnimData.h"
#include <Common/EGame.h>
#include <memory>

class CAnimSetLoader
{
    TResPtr<CAnimSet> pSet;
    EGame mGame{};

    CAnimSetLoader();
    void LoadCorruptionCHAR(IInputStream& rCHAR);
    void LoadReturnsCHAR(IInputStream& rCHAR);
    void LoadPASDatabase(IInputStream& rPAS4);
    void LoadParticleResourceData(IInputStream& rFile, SSetCharacter *pChar, uint16 Version);

    void LoadAnimationSet(IInputStream& rANCS);
    void ProcessPrimitives();

public:
    static std::unique_ptr<CAnimSet> LoadANCS(IInputStream& rANCS, CResourceEntry *pEntry);
    static std::unique_ptr<CAnimSet> LoadCHAR(IInputStream& rCHAR, CResourceEntry *pEntry);
    static std::unique_ptr<CSourceAnimData> LoadSAND(IInputStream& rSAND, CResourceEntry *pEntry);
};

#endif // CCHARACTERLOADER_H
