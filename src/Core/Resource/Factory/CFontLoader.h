#ifndef CFONTLOADER_H
#define CFONTLOADER_H

#include "Core/Resource/CFont.h"
#include "Core/Resource/EFormatVersion.h"
#include "Core/Resource/CResCache.h"

class CFontLoader
{
    TResPtr<CFont> mpFont;
    EGame mVersion;

    CFontLoader();
    CFont* LoadFont(CInputStream& FONT);

public:
    static CFont* LoadFONT(CInputStream& FONT);
    static EGame GetFormatVersion(u32 Version);
};

#endif // CFONTLOADER_H
