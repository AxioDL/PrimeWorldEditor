#ifndef CSCANLOADER_H
#define CSCANLOADER_H

#include "Core/Resource/CScan.h"
#include "Core/Resource/EGame.h"

class CScanLoader
{
    TResPtr<CScan> mpScan;
    CResourceEntry *mpEntry;
    EGame mVersion;

    CScanLoader();
    CScan* LoadScanMP1(IInputStream& rSCAN);
    CScan* LoadScanMP2(IInputStream& rSCAN);
    void LoadParamsMP2(IInputStream& rSCAN);
    void LoadParamsMP3(IInputStream& rSCAN);

public:
    static CScan* LoadSCAN(IInputStream& rSCAN, CResourceEntry *pEntry);
};

#endif // CSCANLOADER_H
