#include "CAudioManager.h"
#include "Core/GameProject/CGameProject.h"
#include "Core/GameProject/CResourceIterator.h"

CAudioManager::CAudioManager(CGameProject *pProj)
    : mpProject(pProj)
{
    ASSERT(mpProject);
}

void CAudioManager::LoadAssets()
{
    // Clear existing assets
    mAudioGroups.clear();
    mpAudioLookupTable = nullptr;
    mpSfxNameList = nullptr;
    mSfxIdMap.clear();

    // Load/sort all audio groups
    for (TResourceIterator<eAudioGroup> It(mpProject->ResourceStore()); It; ++It)
    {
        CAudioGroup *pGroup = (CAudioGroup*) It->Load();
        if (pGroup) mAudioGroups.push_back(pGroup);
    }

    std::sort(mAudioGroups.begin(), mAudioGroups.end(), [](CAudioGroup *pLeft, CAudioGroup *pRight) -> bool {
        return pLeft->GroupID() < pRight->GroupID();
    });

    // Create SFX Define ID -> AGSC map
    for (uint iGrp = 0; iGrp < mAudioGroups.size(); iGrp++)
    {
        CAudioGroup *pGroup = mAudioGroups[iGrp];

        for (uint iSnd = 0; iSnd < pGroup->NumSoundDefineIDs(); iSnd++)
        {
            uint16 DefineID = pGroup->SoundDefineIDByIndex(iSnd);
            ASSERT(mSfxIdMap.find(DefineID) == mSfxIdMap.end());
            mSfxIdMap[DefineID] = pGroup;
        }
    }

    // Load audio lookup table + sfx name list
    TString AudioLookupName = (mpProject->Game() < EGame::EchoesDemo ? "sound_lookup" : "sound_lookup_ATBL");
    CAssetID AudioLookupID = mpProject->FindNamedResource(AudioLookupName);

    if (AudioLookupID.IsValid())
        mpAudioLookupTable = mpProject->ResourceStore()->LoadResource<CAudioLookupTable>(AudioLookupID);

    if (mpProject->Game() >= EGame::EchoesDemo)
    {
        CAssetID SfxNameListID = mpProject->FindNamedResource("audio_name_lookup_STLC");

        if (SfxNameListID.IsValid())
            mpSfxNameList = mpProject->ResourceStore()->LoadResource<CStringList>(SfxNameListID);
    }
}

void CAudioManager::ClearAssets()
{
    mAudioGroups.clear();
    mpAudioLookupTable = nullptr;
    mpSfxNameList = nullptr;
    mSfxIdMap.clear();
}

SSoundInfo CAudioManager::GetSoundInfo(uint32 SoundID)
{
    SSoundInfo Out;
    Out.SoundID = SoundID;
    Out.DefineID = mpAudioLookupTable->FindSoundDefineID(SoundID);
    Out.pAudioGroup = nullptr;

    if (Out.DefineID != 0xFFFF)
    {
        auto Iter = mSfxIdMap.find(Out.DefineID);
        if (Iter != mSfxIdMap.end())
            Out.pAudioGroup = Iter->second;

        if (mpProject->Game() >= EGame::EchoesDemo)
            Out.Name = mpSfxNameList->StringByIndex(Out.DefineID);
    }

    return Out;
}

void CAudioManager::LogSoundInfo(uint32 SoundID)
{
    SSoundInfo SoundInfo = GetSoundInfo(SoundID);

    if (SoundInfo.DefineID != 0xFFFF)
    {
        if (mpProject->Game() >= EGame::EchoesDemo)
            debugf("Sound Name:  %s", *SoundInfo.Name);

        debugf("Sound ID:    0x%04x", SoundInfo.SoundID);
        debugf("Define ID:   0x%04x", SoundInfo.DefineID);
        debugf("Audio Group: %s", *SoundInfo.pAudioGroup->Entry()->Name());
        debugf("");
    }
}
