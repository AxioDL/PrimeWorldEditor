#ifndef CAUDIOMACRO_H
#define CAUDIOMACRO_H

#include "CResource.h"

class CAudioMacro : public CResource
{
    DECLARE_RESOURCE_TYPE(AudioMacro)
    friend class CUnsupportedFormatLoader;

    TString mMacroName;
    std::vector<CAssetID> mSamples;

public:
    CAudioMacro(CResourceEntry *pEntry = 0)
        : CResource(pEntry)
    {}

    virtual CDependencyTree* BuildDependencyTree() const
    {
        CDependencyTree *pTree = new CDependencyTree();

        for (uint32 iSamp = 0; iSamp < mSamples.size(); iSamp++)
            pTree->AddDependency(mSamples[iSamp]);

        return pTree;
    }

    // Accessors
    inline TString MacroName() const                    { return mMacroName; }
    inline uint32 NumSamples() const                    { return mSamples.size(); }
    inline CAssetID SampleByIndex(uint32 Index) const   { return mSamples[Index]; }
};

#endif // CAUDIOMACRO_H
