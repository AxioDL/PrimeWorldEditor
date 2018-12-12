#ifndef CFONTLOADER_H
#define CFONTLOADER_H

#include "Core/GameProject/CResourceStore.h"
#include "Core/Resource/CFont.h"
#include <Common/EGame.h>

class CFontLoader
{
    TResPtr<CFont> mpFont;
    EGame mVersion;

    CFontLoader();
    CFont* LoadFont(IInputStream& rFONT);

public:
    static CFont* LoadFONT(IInputStream& rFONT, CResourceEntry *pEntry);
    static EGame GetFormatVersion(uint32 Version);
};

#endif // CFONTLOADER_H
