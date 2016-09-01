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
    for (TResourceIterator<CAudioGroup> It(mpProject->ResourceStore()); It; ++It)
    {
        CAudioGroup *pGroup = (CAudioGroup*) It->Load();
        if (pGroup) mAudioGroups.push_back(pGroup);
    }

    std::sort(mAudioGroups.begin(), mAudioGroups.end(), [](CAudioGroup *pLeft, CAudioGroup *pRight) -> bool {
        return pLeft->GroupID() < pRight->GroupID();
    });

    // Create SFX Define ID -> AGSC map
    for (u32 iGrp = 0; iGrp < mAudioGroups.size(); iGrp++)
    {
        CAudioGroup *pGroup = mAudioGroups[iGrp];

        for (u32 iSnd = 0; iSnd < pGroup->NumSoundDefineIDs(); iSnd++)
        {
            u16 DefineID = pGroup->SoundDefineIDByIndex(iSnd);
            ASSERT(mSfxIdMap.find(DefineID) == mSfxIdMap.end());
            mSfxIdMap[DefineID] = pGroup;
        }
    }

    // Load audio lookup table + sfx name list
    TString AudioLookupName = (mpProject->Game() < eEchoesDemo ? "sound_lookup" : "sound_lookup_ATBL");
    CAssetID AudioLookupID = mpProject->FindNamedResource(AudioLookupName);

    if (AudioLookupID.IsValid())
        mpAudioLookupTable = mpProject->ResourceStore()->LoadResource(AudioLookupID, "ATBL");

    if (mpProject->Game() >= eEchoesDemo)
    {
        CAssetID SfxNameListID = mpProject->FindNamedResource("audio_name_lookup_STLC");

        if (SfxNameListID.IsValid())
            mpSfxNameList = mpProject->ResourceStore()->LoadResource(SfxNameListID, "STLC");
    }
}

void CAudioManager::LogSoundInfo(u32 SoundID)
{
    u16 DefineID = mpAudioLookupTable->FindSoundDefineID(SoundID);

    if (DefineID == -1)
        Log::Write("Invalid sound");

    else
    {
        auto Iter = mSfxIdMap.find(DefineID);

        if (Iter != mSfxIdMap.end())
        {
            if (mpProject->Game() >= eEchoesDemo)
            {
                TString SoundName = mpSfxNameList->StringByIndex(DefineID);
                Log::Write("Sound Name: " + SoundName);
            }

            CAudioGroup *pGroup = Iter->second;
            Log::Write("Sound ID: " + TString::HexString(SoundID, 4));
            Log::Write("Define ID: " + TString::HexString(DefineID, 4));
            Log::Write("Audio Group: " + pGroup->Entry()->Name().ToUTF8());
            Log::Write("");
        }
    }
}
