#ifndef CMATERIALSET_H
#define CMATERIALSET_H

#include "CMaterial.h"
#include "CTexture.h"
#include "EFormatVersion.h"
#include <FileIO/CInputStream.h>

class CMaterialSet
{
    friend class CMaterialLoader;
    friend class CMaterialCooker;

    std::vector<CMaterial*> mMaterials;

public:
    CMaterialSet();
    ~CMaterialSet();
    CMaterialSet* Clone();
    u32 NumMaterials();
    CMaterial* MaterialByIndex(u32 index);
    CMaterial* MaterialByName(const TString& name);
    u32 MaterialIndexByName(const TString& name);
};

#endif // CMATERIALSET_H
