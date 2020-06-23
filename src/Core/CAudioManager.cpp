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
    for (TResourceIterator<EResourceType::AudioGroup> It(mpProject->ResourceStore()); It; ++It)
    {
        if (auto* pGroup = static_cast<CAudioGroup*>(It->Load()))
            mAudioGroups.emplace_back(pGroup);
    }

    std::sort(mAudioGroups.begin(), mAudioGroups.end(), [](const CAudioGroup *pLeft, const CAudioGroup *pRight) {
        return pLeft->GroupID() < pRight->GroupID();
    });

    // Create SFX Define ID -> AGSC map
    for (CAudioGroup* group : mAudioGroups)
    {
        for (size_t iSnd = 0; iSnd < group->NumSoundDefineIDs(); iSnd++)
        {
            const uint16 DefineID = group->SoundDefineIDByIndex(iSnd);
            ASSERT(mSfxIdMap.find(DefineID) == mSfxIdMap.cend());
            mSfxIdMap.insert_or_assign(DefineID, group);
        }
    }

    // Load audio lookup table + sfx name list
    const std::string_view AudioLookupName = mpProject->Game() < EGame::EchoesDemo ? "sound_lookup" : "sound_lookup_ATBL";
    const CAssetID AudioLookupID = mpProject->FindNamedResource(AudioLookupName);

    if (AudioLookupID.IsValid())
        mpAudioLookupTable = mpProject->ResourceStore()->LoadResource<CAudioLookupTable>(AudioLookupID);

    if (mpProject->Game() >= EGame::EchoesDemo)
    {
        const CAssetID SfxNameListID = mpProject->FindNamedResource("audio_name_lookup_STLC");

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

SSoundInfo CAudioManager::GetSoundInfo(uint32 SoundID) const
{
    SSoundInfo Out;
    Out.SoundID = SoundID;
    Out.DefineID = mpAudioLookupTable->FindSoundDefineID(SoundID);
    Out.pAudioGroup = nullptr;

    if (Out.DefineID != 0xFFFF)
    {
        const auto Iter = mSfxIdMap.find(Out.DefineID);
        if (Iter != mSfxIdMap.cend())
            Out.pAudioGroup = Iter->second;

        if (mpProject->Game() >= EGame::EchoesDemo)
            Out.Name = mpSfxNameList->StringByIndex(Out.DefineID);
    }

    return Out;
}

void CAudioManager::LogSoundInfo(uint32 SoundID) const
{
    const SSoundInfo SoundInfo = GetSoundInfo(SoundID);

    if (SoundInfo.DefineID == 0xFFFF)
        return;

    if (mpProject->Game() >= EGame::EchoesDemo)
        debugf("Sound Name:  %s", *SoundInfo.Name);

    debugf("Sound ID:    0x%04x", SoundInfo.SoundID);
    debugf("Define ID:   0x%04x", SoundInfo.DefineID);
    debugf("Audio Group: %s", *SoundInfo.pAudioGroup->Entry()->Name());
    debugf("");
}
