#ifndef CMATERIALSET_H
#define CMATERIALSET_H

#include "CMaterial.h"
#include "CTexture.h"
#include <Common/EGame.h>
#include <Common/FileIO/IInputStream.h>
#include <algorithm>

class CMaterialSet
{
    friend class CMaterialLoader;
    friend class CMaterialCooker;

    std::vector<std::unique_ptr<CMaterial>> mMaterials;

public:
    CMaterialSet() = default;
    ~CMaterialSet() = default;

    std::unique_ptr<CMaterialSet> Clone()
    {
        auto pOut = std::make_unique<CMaterialSet>();

        pOut->mMaterials.resize(mMaterials.size());
        for (size_t i = 0; i < mMaterials.size(); i++)
            pOut->mMaterials[i] = mMaterials[i]->Clone();

        return pOut;
    }

    size_t NumMaterials() const
    {
        return mMaterials.size();
    }

    CMaterial* MaterialByIndex(size_t Index, bool TryBloom)
    {
        if (Index >= NumMaterials())
            return nullptr;

        CMaterial* Ret = mMaterials[Index].get();
        if (TryBloom && Ret->GetBloomVersion())
            return Ret->GetBloomVersion();

        return Ret;
    }

    CMaterial* MaterialByName(const TString& rkName)
    {
        const auto iter = std::find_if(mMaterials.begin(), mMaterials.end(),
                                       [&rkName](const auto& entry) { return entry->Name() == rkName; });

        if (iter == mMaterials.cend())
            return nullptr;

        return iter->get();
    }

    uint32 MaterialIndexByName(const TString& rkName)
    {
        for (uint32 i = 0; i < mMaterials.size(); i++)
        {
            if (mMaterials[i]->Name() == rkName)
                return i;
        }

        return UINT32_MAX;
    }

    void GetUsedTextureIDs(std::set<CAssetID>& rOut)
    {
        for (const auto& material : mMaterials)
        {
            if (material->IndTexture())
                rOut.insert(material->IndTexture()->ID());

            for (size_t i = 0; i < material->PassCount(); i++)
            {
                const CTexture *pTex = material->Pass(i)->Texture();
                if (pTex)
                    rOut.insert(pTex->ID());
            }
        }
    }
};

#endif // CMATERIALSET_H
