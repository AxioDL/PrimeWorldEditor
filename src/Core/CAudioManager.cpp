#include "Core/CAudioManager.h"

#include "Core/GameProject/CGameProject.h"
#include "Core/Resource/CAudioGroup.h"
#include "Core/Resource/CAudioLookupTable.h"
#include "Core/Resource/CStringList.h"
#include <Common/Log.h>

#include <algorithm>

CAudioManager::CAudioManager(CGameProject *pProj)
    : mpProject(pProj)
{
    ASSERT(mpProject);
}

CAudioManager::~CAudioManager() = default;

void CAudioManager::LoadAssets()
{
    // Clear existing assets
    mAudioGroups.clear();
    mpAudioLookupTable = nullptr;
    mpSfxNameList = nullptr;
    mSfxIdMap.clear();

    // Load/sort all audio groups
    for (const auto& entry : mpProject->ResourceStore()->MakeTypedResourceView(EResourceType::AudioGroup))
    {
        if (auto* group = static_cast<CAudioGroup*>(entry->Load()))
            mAudioGroups.emplace_back(group);
    }

    std::ranges::sort(mAudioGroups, [](const CAudioGroup* pLeft, const CAudioGroup* pRight) {
        return pLeft->GroupID() < pRight->GroupID();
    });

    // Create SFX Define ID -> AGSC map
    for (CAudioGroup* group : mAudioGroups)
    {
        for (const auto DefineID : group->SoundDefines())
        {
            ASSERT(!mSfxIdMap.contains(DefineID));
            mSfxIdMap.insert_or_assign(DefineID, group);
        }
    }

    // Load audio lookup table + sfx name list
    const std::string_view AudioLookupName = mpProject->Game() < EGame::EchoesDemo ? "sound_lookup" : "sound_lookup_ATBL";
    const CAssetID AudioLookupID = mpProject->FindNamedResource(AudioLookupName);

    if (AudioLookupID.IsValid())
        mpAudioLookupTable = mpProject->ResourceStore()->LoadResource<CAudioLookupTable>(AudioLookupID);

    if (mpProject->Game() >= EGame::Prime)
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

SSoundInfo CAudioManager::GetSoundInfo(uint32_t SoundID) const
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

        if (mpProject->Game() >= EGame::Prime && mpSfxNameList != nullptr)
            Out.Name = mpSfxNameList->StringByIndex(Out.DefineID);
    }

    return Out;
}

void CAudioManager::LogSoundInfo(uint32_t SoundID) const
{
    const SSoundInfo SoundInfo = GetSoundInfo(SoundID);

    if (SoundInfo.DefineID == 0xFFFF)
        return;

    if (mpProject->Game() >= EGame::EchoesDemo)
        NLog::Debug("Sound Name:  {}", SoundInfo.Name);

    NLog::Debug("Sound ID:    0x{:04x}", SoundInfo.SoundID);
    NLog::Debug("Define ID:   0x{:04x}", SoundInfo.DefineID);
    NLog::Debug("Audio Group: {}\n", SoundInfo.pAudioGroup->Entry()->Name());
}
