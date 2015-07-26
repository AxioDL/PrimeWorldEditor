#ifndef CMATERIALSET_H
#define CMATERIALSET_H

#include <FileIO/CInputStream.h>
#include <Resource/CTexture.h>
#include <Resource/CMaterial.h>
#include "EFormatVersion.h"

class CMaterialSet
{
public:
    std::vector<CTexture*> textures;
    std::vector<CMaterial*> materials;

    CMaterialSet();
    ~CMaterialSet();
};

#endif // CMATERIALSET_H
