#ifndef CSCAN_H
#define CSCAN_H

#include "CResource.h"
#include "CStringTable.h"
#include "TResPtr.h"
#include "Core/Resource/Animation/CAnimationParameters.h"
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

    struct SScanInfoSecondaryModel
    {
        CAssetID ModelID;
        CAnimationParameters AnimParams;
        TString AttachBoneName;
    };

private:
    // Common
    TResPtr<CStringTable> mpStringTable;
    bool mIsSlow;
    bool mIsImportant;
    ELogbookCategory mCategory;

    // MP1
    CAssetID mFrameID;
    CAssetID mScanImageTextures[4];

    // MP2/3
    bool mUseLogbookModelPostScan;
    CAssetID mPostOverrideTexture;
    float mLogbookDefaultRotX;
    float mLogbookDefaultRotZ;
    float mLogbookScale;
    CAssetID mLogbookModel;
    CAnimationParameters mLogbookAnimParams;
    CAnimationParameters mUnknownAnimParams;
    std::vector<SScanInfoSecondaryModel> mSecondaryModels;

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
        // note: not handling Corruption yet
        if (Game() >= eCorruptionProto)
            Log::Warning("CScan::BuildDependencyTree not handling Corruption dependencies");

        CDependencyTree *pTree = new CDependencyTree(ID());

        if (Game() <= ePrime)
            pTree->AddDependency(mFrameID);

        pTree->AddDependency(mpStringTable);

        if (Game() <= ePrime)
        {
            for (u32 iImg = 0; iImg < 4; iImg++)
                pTree->AddDependency(mScanImageTextures[iImg]);
        }

        else if (Game() <= eEchoes)
        {
            pTree->AddDependency(mPostOverrideTexture);
            pTree->AddDependency(mLogbookModel);
            pTree->AddCharacterDependency(mLogbookAnimParams);
            pTree->AddCharacterDependency(mUnknownAnimParams);

            for (u32 iSec = 0; iSec < mSecondaryModels.size(); iSec++)
            {
                const SScanInfoSecondaryModel& rkSecModel = mSecondaryModels[iSec];
                pTree->AddDependency(rkSecModel.ModelID);
                pTree->AddCharacterDependency(rkSecModel.AnimParams);
            }
        }

        return pTree;
    }

    // Accessors
    inline CStringTable* ScanText() const           { return mpStringTable; }
    inline bool IsImportant() const                 { return mIsImportant; }
    inline bool IsSlow() const                      { return mIsSlow; }
    inline ELogbookCategory LogbookCategory() const { return mCategory; }
    inline CAssetID GuiFrame() const                { return mFrameID; }
    inline CAssetID ScanImage(u32 ImgIndex) const   { return mScanImageTextures[ImgIndex]; }
    inline CAssetID LogbookDisplayAssetID() const   { return (mLogbookAnimParams.ID().IsValid() ? mLogbookAnimParams.ID() : mLogbookModel); }
};

#endif // CSCAN_H
