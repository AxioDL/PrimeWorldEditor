#ifndef CSCAN_H
#define CSCAN_H

#include "CResource.h"
#include "CStringTable.h"
#include "EFormatVersion.h"
#include <Core/TResPtr.h>

class CScan : public CResource
{
    DECLARE_RESOURCE_TYPE(eScan)
    friend class CScanLoader;

public:
    // This likely needs revising when MP2/MP3 support is added
    enum ELogbookCategory
    {
        eNone,
        ePirateData,
        eChozoLore,
        eCreatures,
        eResearch
    };

private:
    EGame mVersion;
    TResPtr<CResource> mpFrame;
    TResPtr<CStringTable> mpStringTable;
    bool mIsSlow;
    bool mIsImportant;
    ELogbookCategory mCategory;

public:
    CScan();
    ~CScan();
    EGame Version();
    CStringTable* ScanText();
    bool IsImportant();
    bool IsSlow();
    ELogbookCategory LogbookCategory();
};

#endif // CSCAN_H
