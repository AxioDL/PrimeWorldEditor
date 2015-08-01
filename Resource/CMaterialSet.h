#ifndef CMATERIALSET_H
#define CMATERIALSET_H

#include <FileIO/CInputStream.h>
#include <Resource/CTexture.h>
#include <Resource/CMaterial.h>
#include "EFormatVersion.h"

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
    CMaterial* MaterialByName(const std::string& name);
};

#endif // CMATERIALSET_H
