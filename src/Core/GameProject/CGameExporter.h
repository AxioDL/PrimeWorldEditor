#ifndef CGAMEEXPORTER_H
#define CGAMEEXPORTER_H

#include "CGameProject.h"
#include <Common/CUniqueID.h>
#include <Common/Flags.h>
#include <Common/TString.h>
#include <Common/types.h>
#include <map>

class CGameExporter
{
    // Project
    CGameProject *mpProject;

    // Directories
    TWideString mGameDir;
    TWideString mExportDir;
    TWideString mDiscDir;
    TWideString mCookedResDir;
    TWideString mCookedWorldsDir;
    TWideString mRawResDir;
    TWideString mRawWorldsDir;

    // Resources
    TWideStringList mWorldPaks;
    TWideStringList mResourcePaks;

    struct SResourceInstance
    {
        TWideString PakFile;
        CUniqueID ResourceID;
        CFourCC ResourceType;
        u32 PakOffset;
        u32 PakSize;
        bool Compressed;
    };
    std::map<u64, SResourceInstance> mResourceMap;

public:
    CGameExporter(const TString& rkInputDir, const TString& rkOutputDir);
    bool Export();

protected:
    void CopyDiscData();
    void LoadPaks();
    void LoadPakResource(const SResourceInstance& rkResource, std::vector<u8>& rBuffer);
    void ExportCookedResources();

    inline EGame Game() const       { return mpProject->Game(); }
    inline void SetGame(EGame Game) { mpProject->SetGame(Game); }
};

#endif // CGAMEEXPORTER_H
