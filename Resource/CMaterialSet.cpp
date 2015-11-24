#include "CMaterialSet.h"
#include <Core/CResCache.h>
#include <iostream>

CMaterialSet::CMaterialSet()
{
}

CMaterialSet::~CMaterialSet()
{
    for (u32 iMat = 0; iMat < mMaterials.size(); iMat++)
        delete mMaterials[iMat];
}

CMaterialSet* CMaterialSet::Clone()
{
    CMaterialSet *pOut = new CMaterialSet();

    pOut->mMaterials.resize(mMaterials.size());
    for (u32 iMat = 0; iMat < mMaterials.size(); iMat++)
        pOut->mMaterials[iMat] = mMaterials[iMat]->Clone();

    return pOut;
}

u32 CMaterialSet::NumMaterials()
{
    return mMaterials.size();
}

CMaterial* CMaterialSet::MaterialByIndex(u32 index)
{
    if (index >= NumMaterials()) return nullptr;
    return mMaterials[index];
}

CMaterial* CMaterialSet::MaterialByName(const TString& name)
{
    for (auto it = mMaterials.begin(); it != mMaterials.end(); it++)
        if ((*it)->Name() == name) return *it;
    return nullptr;
}

u32 CMaterialSet::MaterialIndexByName(const TString& name)
{
    for (u32 iMat = 0; iMat < mMaterials.size(); iMat++)
        if (mMaterials[iMat]->Name() == name) return iMat;
    return -1;
}
