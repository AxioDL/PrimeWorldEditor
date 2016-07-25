#ifndef CSCAN_H
#define CSCAN_H

#include "CResource.h"
#include "CStringTable.h"
#include "EGame.h"
#include "TResPtr.h"

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
    CScan(CResourceEntry *pEntry = 0)
        : CResource(pEntry)
        , mpFrame(nullptr)
        , mpStringTable(nullptr)
        , mIsSlow(false)
        , mIsImportant(false)
        , mCategory(eNone)
    {}

    CDependencyTree* BuildDependencyTree() const
    {
        if (Game() >= eEchoesDemo)
            Log::Warning("CScan::BuildDependencyTree not handling Echoes/Corruption dependencies");

        CDependencyTree *pTree = new CDependencyTree(ResID());
        pTree->AddDependency(mpFrame);
        pTree->AddDependency(mpStringTable);
        return pTree;
    }

    EGame Version() const                       { return mVersion; }
    CStringTable* ScanText() const              { return mpStringTable; }
    bool IsImportant() const                    { return mIsImportant; }
    bool IsSlow() const                         { return mIsSlow; }
    ELogbookCategory LogbookCategory() const    { return mCategory; }
};

#endif // CSCAN_H
