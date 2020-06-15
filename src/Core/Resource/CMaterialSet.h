#ifndef CMATERIALSET_H
#define CMATERIALSET_H

#include "CMaterial.h"
#include "CTexture.h"
#include <Common/EGame.h>
#include <Common/FileIO/IInputStream.h>

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
        for (uint32 iMat = 0; iMat < mMaterials.size(); iMat++)
            pOut->mMaterials[iMat] = mMaterials[iMat]->Clone();

        return pOut;
    }

    uint32 NumMaterials() const
    {
        return mMaterials.size();
    }

    CMaterial* MaterialByIndex(uint32 Index, bool TryBloom)
    {
        if (Index >= NumMaterials()) return nullptr;
        CMaterial* Ret = mMaterials[Index].get();
        if (TryBloom && Ret->GetBloomVersion())
            return Ret->GetBloomVersion();
        return Ret;
    }

    CMaterial* MaterialByName(const TString& rkName)
    {
        for (auto it = mMaterials.begin(); it != mMaterials.end(); it++)
            if ((*it)->Name() == rkName) return it->get();

        return nullptr;
    }

    uint32 MaterialIndexByName(const TString& rkName)
    {
        for (uint32 iMat = 0; iMat < mMaterials.size(); iMat++)
            if (mMaterials[iMat]->Name() == rkName) return iMat;

        return -1;
    }

    void GetUsedTextureIDs(std::set<CAssetID>& rOut)
    {
        for (uint32 iMat = 0; iMat < mMaterials.size(); iMat++)
        {
            CMaterial *pMat = mMaterials[iMat].get();
            if (pMat->IndTexture()) rOut.insert(pMat->IndTexture()->ID());

            for (uint32 iPass = 0; iPass < pMat->PassCount(); iPass++)
            {
                CTexture *pTex = pMat->Pass(iPass)->Texture();
                if (pTex) rOut.insert(pTex->ID());
            }
        }
    }
};

#endif // CMATERIALSET_H
