#ifndef CGAMEEXPORTER_H
#define CGAMEEXPORTER_H

#include "CAssetNameMap.h"
#include "CGameProject.h"
#include "CResourceStore.h"
#include <Common/CAssetID.h>
#include <Common/Flags.h>
#include <Common/TString.h>
#include <Common/types.h>
#include <map>

class CGameExporter
{
    // Project
    CGameProject *mpProject;
    CResourceStore *mpStore;
    EGame mGame;

    // Directories
    TWideString mGameDir;
    TWideString mExportDir;
    TWideString mDiscDir;
    TWideString mContentDir;
    TWideString mCookedDir;

    TWideString mWorldsDirName;

    // Resources
    TWideStringList mPaks;
    std::map<CAssetID, bool> mAreaDuplicateMap;
    CAssetNameMap *mpNameMap;

    struct SResourceInstance
    {
        TWideString PakFile;
        CAssetID ResourceID;
        CFourCC ResourceType;
        u32 PakOffset;
        u32 PakSize;
        bool Compressed;
        bool Exported;
    };
    std::map<CAssetID, SResourceInstance> mResourceMap;

public:
    CGameExporter(const TString& rkInputDir, const TString& rkOutputDir);
    bool Export();
    void LoadResource(const CAssetID& rkID, std::vector<u8>& rBuffer);

protected:
    void CopyDiscData();
    void LoadPaks();
    void LoadResource(const SResourceInstance& rkResource, std::vector<u8>& rBuffer);
    void ExportCookedResources();
    void ExportResourceEditorData();
    void ExportResource(SResourceInstance& rRes);

    // Convenience Functions
    inline SResourceInstance* FindResourceInstance(const CAssetID& rkID)
    {
        u64 IntegralID = rkID.ToLongLong();
        auto Found = mResourceMap.find(IntegralID);
        return (Found == mResourceMap.end() ? nullptr : &Found->second);
    }
};

#endif // CGAMEEXPORTER_H
