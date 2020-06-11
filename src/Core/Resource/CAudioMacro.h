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
    explicit CAudioMacro(CResourceEntry *pEntry = nullptr)
        : CResource(pEntry)
    {}

    CDependencyTree* BuildDependencyTree() const override
    {
        auto *pTree = new CDependencyTree();

        for (const auto& sample : mSamples)
            pTree->AddDependency(sample);

        return pTree;
    }

    // Accessors
    TString MacroName() const                    { return mMacroName; }
    uint32 NumSamples() const                    { return mSamples.size(); }
    CAssetID SampleByIndex(uint32 Index) const   { return mSamples[Index]; }
};

#endif // CAUDIOMACRO_H
