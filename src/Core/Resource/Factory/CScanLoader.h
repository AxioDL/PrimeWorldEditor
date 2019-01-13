#ifndef CSCANLOADER_H
#define CSCANLOADER_H

#include "Core/Resource/Scan/CScan.h"
#include <Common/EGame.h>

class CScanLoader
{
    TResPtr<CScan> mpScan;

    CScanLoader() {}
    CScan* LoadScanMP1(IInputStream& SCAN,  CResourceEntry* pEntry);
    CScan* LoadScanMP2(IInputStream& SCAN,  CResourceEntry* pEntry);

public:
    static CScan* LoadSCAN(IInputStream& rSCAN, CResourceEntry *pEntry);
};

#endif // CSCANLOADER_H
