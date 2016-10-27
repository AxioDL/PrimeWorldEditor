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
    void LoadPASDatabase(IInputStream& rPAS4);

    // Animation Set load functions
    struct SPrimitive
    {
        u32 Index;
        u32 AnimID;
        TString Name;
        bool Loaded;

        SPrimitive() : Loaded(false) {}
    };
    std::vector<SPrimitive> mAnimPrimitives;

    void LoadAnimation(IInputStream& rANCS);
    void LoadMetaAnimation(IInputStream& rANCS);
    void LoadPrimitive(IInputStream& rANCS);
    void LoadBlend(IInputStream& rANCS);
    void LoadRandom(IInputStream& rANCS);
    void LoadAnimProbabilityPair(IInputStream& rANCS);
    void LoadSequence(IInputStream& rANCS);
    void LoadTransition(IInputStream& rANCS);
    void LoadMetaTransition(IInputStream& rANCS);
    void LoadTransAnimation(IInputStream& rANCS);
    void LoadTransTransition(IInputStream& rANCS);
    void LoadAdditiveAnimation(IInputStream& rANCS);
    void LoadHalfTransition(IInputStream& rANCS);

public:
    static CAnimSet* LoadANCSOrCHAR(IInputStream& rFile, CResourceEntry *pEntry);
    static CAnimSet* LoadANCS(IInputStream& rANCS, CResourceEntry *pEntry);
    static CAnimSet* LoadCHAR(IInputStream& rCHAR, CResourceEntry *pEntry);
};

#endif // CCHARACTERLOADER_H
