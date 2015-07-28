#ifndef CSCAN_H
#define CSCAN_H

#include "CResource.h"
#include "CStringTable.h"
#include "EFormatVersion.h"
#include <Core/CToken.h>

class CScan : public CResource
{
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
    CResource *mpFrame;
    CStringTable *mpStringTable;
    CToken mFrameToken;
    CToken mStringToken;
    bool mIsSlow;
    bool mIsImportant;
    ELogbookCategory mCategory;

public:
    CScan();
    ~CScan();
    EResType Type();
    EGame Version();
    CStringTable* ScanText();
    bool IsImportant();
    bool IsSlow();
    ELogbookCategory LogbookCategory();
};

#endif // CSCAN_H
