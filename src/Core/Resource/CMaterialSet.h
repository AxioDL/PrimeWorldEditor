#ifndef CMATERIALSET_H
#define CMATERIALSET_H

#include "CMaterial.h"
#include "CTexture.h"
#include <FileIO/IInputStream.h>
#include <Common/EGame.h>

class CMaterialSet
{
    friend class CMaterialLoader;
    friend class CMaterialCooker;

    std::vector<CMaterial*> mMaterials;

public:
    CMaterialSet() {}

    ~CMaterialSet()
    {
        for (u32 iMat = 0; iMat < mMaterials.size(); iMat++)
            delete mMaterials[iMat];
    }

    CMaterialSet* Clone()
    {
        CMaterialSet *pOut = new CMaterialSet();

        pOut->mMaterials.resize(mMaterials.size());
        for (u32 iMat = 0; iMat < mMaterials.size(); iMat++)
            pOut->mMaterials[iMat] = mMaterials[iMat]->Clone();

        return pOut;
    }

    u32 NumMaterials()
    {
        return mMaterials.size();
    }

    CMaterial* MaterialByIndex(u32 Index)
    {
        if (Index >= NumMaterials()) return nullptr;
        return mMaterials[Index];
    }

    CMaterial* MaterialByName(const TString& rkName)
    {
        for (auto it = mMaterials.begin(); it != mMaterials.end(); it++)
            if ((*it)->Name() == rkName) return *it;

        return nullptr;
    }

    u32 MaterialIndexByName(const TString& rkName)
    {
        for (u32 iMat = 0; iMat < mMaterials.size(); iMat++)
            if (mMaterials[iMat]->Name() == rkName) return iMat;

        return -1;
    }

    void GetUsedTextureIDs(std::set<CAssetID>& rOut)
    {
        for (u32 iMat = 0; iMat < mMaterials.size(); iMat++)
        {
            CMaterial *pMat = mMaterials[iMat];
            if (pMat->IndTexture()) rOut.insert(pMat->IndTexture()->ID());

            for (u32 iPass = 0; iPass < pMat->PassCount(); iPass++)
            {
                CTexture *pTex = pMat->Pass(iPass)->Texture();
                if (pTex) rOut.insert(pTex->ID());
            }
        }
    }
};

#endif // CMATERIALSET_H
