#ifndef CSCANLOADER_H
#define CSCANLOADER_H

#include "../CScan.h"
#include "../EFormatVersion.h"

class CScanLoader
{
    TResPtr<CScan> mpScan;
    EGame mVersion;

    CScanLoader();
    CScan* LoadScanMP1(CInputStream& SCAN);
    CScan* LoadScanMP2(CInputStream& SCAN);
    void LoadParamsMP2(CInputStream& SCAN);
    void LoadParamsMP3(CInputStream& SCAN);

public:
    static CScan* LoadSCAN(CInputStream& SCAN);
};

#endif // CSCANLOADER_H
