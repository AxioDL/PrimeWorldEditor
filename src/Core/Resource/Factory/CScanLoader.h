#ifndef CSCANLOADER_H
#define CSCANLOADER_H

#include "Core/Resource/CScan.h"
#include "Core/Resource/EFormatVersion.h"

class CScanLoader
{
    TResPtr<CScan> mpScan;
    EGame mVersion;

    CScanLoader();
    CScan* LoadScanMP1(IInputStream& SCAN);
    CScan* LoadScanMP2(IInputStream& SCAN);
    void LoadParamsMP2(IInputStream& SCAN);
    void LoadParamsMP3(IInputStream& SCAN);

public:
    static CScan* LoadSCAN(IInputStream& SCAN);
};

#endif // CSCANLOADER_H
