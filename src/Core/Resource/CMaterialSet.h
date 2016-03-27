#ifndef CMATERIALSET_H
#define CMATERIALSET_H

#include "CMaterial.h"
#include "CTexture.h"
#include "EGame.h"
#include <FileIO/IInputStream.h>

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
};

#endif // CMATERIALSET_H
