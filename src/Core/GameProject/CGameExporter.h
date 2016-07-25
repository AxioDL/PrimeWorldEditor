#ifndef CGAMEEXPORTER_H
#define CGAMEEXPORTER_H

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
    CResourceStore mStore;

    // Directories
    TWideString mGameDir;
    TWideString mExportDir;
    TWideString mDiscDir;
    TWideString mContentDir;
    TWideString mCookedDir;

    TWideString mWorldsDirName;

    // Resources
    TWideStringList mPaks;

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
    std::map<u64, SResourceInstance> mResourceMap;

    struct SResourcePath
    {
        TWideString Dir;
        TWideString Name;
    };
    std::map<u64, SResourcePath> mResourcePaths;

public:
    CGameExporter(const TString& rkInputDir, const TString& rkOutputDir);
    bool Export();
    void LoadResource(const CAssetID& rkID, std::vector<u8>& rBuffer);

protected:
    void CopyDiscData();
    void LoadAssetList();
    void LoadPaks();
    void LoadResource(const SResourceInstance& rkResource, std::vector<u8>& rBuffer);
    void ExportWorlds();
    void ExportCookedResources();
    void ExportResource(SResourceInstance& rRes);

    // Convenience Functions
    inline SResourceInstance* FindResourceInstance(const CAssetID& rkID)
    {
        u64 IntegralID = rkID.ToLongLong();
        auto Found = mResourceMap.find(IntegralID);
        return (Found == mResourceMap.end() ? nullptr : &Found->second);
    }

    inline SResourcePath* FindResourcePath(const CAssetID& rkID)
    {
        u64 IntegralID = rkID.ToLongLong();
        auto Found = mResourcePaths.find(IntegralID);
        return (Found == mResourcePaths.end() ? nullptr : &Found->second);
    }

    inline void SetResourcePath(const CAssetID& rkID, const TWideString& rkDir, const TWideString& rkName)
    {
        SetResourcePath(rkID.ToLongLong(), rkDir, rkName);
    }

    inline void SetResourcePath(u64 ID, const TWideString& rkDir, const TWideString& rkName)
    {
        mResourcePaths[ID] = SResourcePath { rkDir, rkName };
    }

    inline EGame Game() const       { return mpProject->Game(); }
    inline void SetGame(EGame Game) { mpProject->SetGame(Game); }
};

#endif // CGAMEEXPORTER_H
