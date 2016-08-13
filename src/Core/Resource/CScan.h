#ifndef CSCAN_H
#define CSCAN_H

#include "CResource.h"
#include "CStringTable.h"
#include "TResPtr.h"
#include <Common/EGame.h>

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
    CAssetID mFrameID;
    TResPtr<CStringTable> mpStringTable;
    bool mIsSlow;
    bool mIsImportant;
    ELogbookCategory mCategory;
    CAssetID mScanImageTextures[4];

public:
    CScan(CResourceEntry *pEntry = 0)
        : CResource(pEntry)
        , mpStringTable(nullptr)
        , mIsSlow(false)
        , mIsImportant(false)
        , mCategory(eNone)
    {}

    CDependencyTree* BuildDependencyTree() const
    {
        if (Game() >= eEchoesDemo)
            Log::Warning("CScan::BuildDependencyTree not handling Echoes/Corruption dependencies");

        CDependencyTree *pTree = new CDependencyTree(ID());
        pTree->AddDependency(mFrameID);
        pTree->AddDependency(mpStringTable);

        for (u32 iImg = 0; iImg < 4; iImg++)
            pTree->AddDependency(mScanImageTextures[iImg]);

        return pTree;
    }

    EGame Version() const                       { return mVersion; }
    CStringTable* ScanText() const              { return mpStringTable; }
    bool IsImportant() const                    { return mIsImportant; }
    bool IsSlow() const                         { return mIsSlow; }
    ELogbookCategory LogbookCategory() const    { return mCategory; }
};

#endif // CSCAN_H
