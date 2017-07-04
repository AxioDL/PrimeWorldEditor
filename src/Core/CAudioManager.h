#ifndef CAUDIOMANAGER
#define CAUDIOMANAGER

#include "Core/Resource/CAudioGroup.h"
#include "Core/Resource/CAudioLookupTable.h"
#include "Core/Resource/CStringList.h"
#include "Core/Resource/TResPtr.h"
#include <algorithm>
#include <unordered_map>

struct SSoundInfo
{
    CAudioGroup *pAudioGroup;
    TString Name;
    u32 SoundID;
    u16 DefineID;
};

class CAudioManager
{
    CGameProject *mpProject;

    std::vector<TResPtr<CAudioGroup>> mAudioGroups;
    TResPtr<CAudioLookupTable> mpAudioLookupTable;
    TResPtr<CStringList> mpSfxNameList;
    std::unordered_map<u16, CAudioGroup*> mSfxIdMap;

public:
    CAudioManager(CGameProject *pProj);
    void LoadAssets();
    void ClearAssets();
    SSoundInfo GetSoundInfo(u32 SoundID);
    void LogSoundInfo(u32 SoundID);
};

#endif // CAUDIOMANAGER

