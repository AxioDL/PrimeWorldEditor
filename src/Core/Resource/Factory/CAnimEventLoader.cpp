#include "CAnimEventLoader.h"
#include "Core/CAudioManager.h"
#include "Core/GameProject/CGameProject.h"

void CAnimEventLoader::LoadEvents(IInputStream& rEVNT)
{
    u32 Version = rEVNT.ReadLong();
    ASSERT(Version == 1 || Version == 2);

    // Loop Events
    u32 NumLoopEvents = rEVNT.ReadLong();

    for (u32 iLoop = 0; iLoop < NumLoopEvents; iLoop++)
    {
        LoadLoopEvent(rEVNT);
    }

    // User Events
    u32 NumUserEvents = rEVNT.ReadLong();

    for (u32 iUser = 0; iUser < NumUserEvents; iUser++)
    {
        LoadUserEvent(rEVNT);
    }

    // Effect Events
    u32 NumEffectEvents = rEVNT.ReadLong();

    for (u32 iFX = 0; iFX < NumEffectEvents; iFX++)
    {
        LoadEffectEvent(rEVNT);
    }

    // Sound Events
    if (Version == 2)
    {
        u32 NumSoundEvents = rEVNT.ReadLong();

        for (u32 iSound = 0; iSound < NumSoundEvents; iSound++)
        {
            LoadSoundEvent(rEVNT);
        }
    }
}

s32 CAnimEventLoader::LoadEventBase(IInputStream& rEVNT)
{
    rEVNT.Skip(0x2);
    rEVNT.ReadString();
    rEVNT.Skip(mGame < EGame::CorruptionProto ? 0x13 : 0x17);
    s32 CharacterIndex = rEVNT.ReadLong();
    rEVNT.Skip(mGame < EGame::CorruptionProto ? 0x4 : 0x18);
    return CharacterIndex;
}

void CAnimEventLoader::LoadLoopEvent(IInputStream& rEVNT)
{
    LoadEventBase(rEVNT);
    rEVNT.Skip(0x1);
}

void CAnimEventLoader::LoadUserEvent(IInputStream& rEVNT)
{
    LoadEventBase(rEVNT);
    rEVNT.Skip(0x4);
    rEVNT.ReadString();
}

void CAnimEventLoader::LoadEffectEvent(IInputStream& rEVNT)
{
    s32 CharIndex = LoadEventBase(rEVNT);
    rEVNT.Skip(mGame < EGame::CorruptionProto ? 0x8 : 0x4);
    CAssetID ParticleID(rEVNT, mGame);
    mpEventData->AddEvent(CharIndex, ParticleID);

    if (mGame <= EGame::Prime)
        rEVNT.ReadString();
    else if (mGame <= EGame::Echoes)
        rEVNT.Skip(0x4);

    rEVNT.Skip(0x8);
}

void CAnimEventLoader::LoadSoundEvent(IInputStream& rEVNT)
{
    s32 CharIndex = LoadEventBase(rEVNT);

    // Metroid Prime 1/2
    if (mGame <= EGame::Echoes)
    {
        u32 SoundID = rEVNT.ReadLong() & 0xFFFF;
        rEVNT.Skip(0x8);
        if (mGame >= EGame::Echoes) rEVNT.Skip(0xC);

        if (SoundID != 0xFFFF)
        {
            SSoundInfo SoundInfo = gpResourceStore->Project()->AudioManager()->GetSoundInfo(SoundID);

            if (SoundInfo.pAudioGroup)
                mpEventData->AddEvent(CharIndex, SoundInfo.pAudioGroup->ID());
        }
    }

    // Metroid Prime 3
    else
    {
        CAssetID SoundID(rEVNT, mGame);
        mpEventData->AddEvent(CharIndex, SoundID);
        rEVNT.Skip(0x8);

        for (u32 StructIdx = 0; StructIdx < 2; StructIdx++)
        {
            u32 StructType = rEVNT.ReadLong();
            ASSERT(StructType <= 2);

            if (StructType == 1)
            {
                rEVNT.Skip(4);
            }
            else if (StructType == 2)
            {
                // This is a maya spline
                rEVNT.Skip(2);
                u32 KnotCount = rEVNT.ReadLong();
                rEVNT.Skip(0xA * KnotCount);
                rEVNT.Skip(9);
            }
        }
    }
}

// ************ STATIC ************
CAnimEventData* CAnimEventLoader::LoadEVNT(IInputStream& rEVNT, CResourceEntry *pEntry)
{
    CAnimEventLoader Loader;
    Loader.mpEventData = new CAnimEventData(pEntry);
    Loader.mGame = EGame::Prime;
    Loader.LoadEvents(rEVNT);
    return Loader.mpEventData;
}

CAnimEventData* CAnimEventLoader::LoadAnimSetEvents(IInputStream& rANCS)
{
    CAnimEventLoader Loader;
    Loader.mpEventData = new CAnimEventData();
    Loader.mGame = EGame::Echoes;
    Loader.LoadEvents(rANCS);
    return Loader.mpEventData;
}

CAnimEventData* CAnimEventLoader::LoadCorruptionCharacterEventSet(IInputStream& rCHAR)
{
    CAnimEventLoader Loader;
    Loader.mpEventData = new CAnimEventData();
    Loader.mGame = EGame::Corruption;

    // Read event set header
    rCHAR.Skip(0x4); // Skip animation ID
    rCHAR.ReadString(); // Skip set name

    // Read effect events
    u32 NumEffectEvents = rCHAR.ReadLong();

    for (u32 EventIdx = 0; EventIdx < NumEffectEvents; EventIdx++)
    {
        rCHAR.ReadString();
        Loader.LoadEffectEvent(rCHAR);
    }

    // Read sound events
    u32 NumSoundEvents = rCHAR.ReadLong();

    for (u32 EventIdx = 0; EventIdx < NumSoundEvents; EventIdx++)
    {
        rCHAR.ReadString();
        Loader.LoadSoundEvent(rCHAR);
    }

    return Loader.mpEventData;
}
