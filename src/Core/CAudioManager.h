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
    uint32 SoundID;
    uint16 DefineID;
};

class CAudioManager
{
    CGameProject *mpProject;

    std::vector<TResPtr<CAudioGroup>> mAudioGroups;
    TResPtr<CAudioLookupTable> mpAudioLookupTable;
    TResPtr<CStringList> mpSfxNameList;
    std::unordered_map<uint16, CAudioGroup*> mSfxIdMap;

public:
    explicit CAudioManager(CGameProject *pProj);
    void LoadAssets();
    void ClearAssets();
    SSoundInfo GetSoundInfo(uint32 SoundID) const;
    void LogSoundInfo(uint32 SoundID) const;
};

#endif // CAUDIOMANAGER

