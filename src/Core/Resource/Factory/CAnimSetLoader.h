#ifndef CCHARACTERLOADER_H
#define CCHARACTERLOADER_H

#include "Core/Resource/CAnimSet.h"
#include "Core/Resource/EGame.h"

class CAnimSetLoader
{
    TResPtr<CAnimSet> pSet;
    CResCache *mpResCache;
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
    static CAnimSet* LoadANCS(IInputStream& rANCS);
    static CAnimSet* LoadCHAR(IInputStream& rCHAR);
};

#endif // CCHARACTERLOADER_H
