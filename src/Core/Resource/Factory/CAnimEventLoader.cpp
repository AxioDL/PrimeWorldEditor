#include "CAnimEventLoader.h"
#include "Core/CAudioManager.h"
#include "Core/GameProject/CGameProject.h"

void CAnimEventLoader::LoadEvents(IInputStream& rEVNT, bool IsEchoes)
{
    u32 Version = rEVNT.ReadLong();
    ASSERT(Version == 1 || Version == 2);

    // Loop Events
    u32 NumLoopEvents = rEVNT.ReadLong();

    for (u32 iLoop = 0; iLoop < NumLoopEvents; iLoop++)
    {
        rEVNT.Seek(0x2, SEEK_CUR);
        rEVNT.ReadString();
        rEVNT.Seek(0x1C, SEEK_CUR);
    }

    // User Events
    u32 NumUserEvents = rEVNT.ReadLong();

    for (u32 iUser = 0; iUser < NumUserEvents; iUser++)
    {
        rEVNT.Seek(0x2, SEEK_CUR);
        rEVNT.ReadString();
        rEVNT.Seek(0x1F, SEEK_CUR);
        rEVNT.ReadString();
    }

    // Effect Events
    u32 NumEffectEvents = rEVNT.ReadLong();

    for (u32 iFX = 0; iFX < NumEffectEvents; iFX++)
    {
        rEVNT.Seek(0x2, SEEK_CUR);
        rEVNT.ReadString();
        rEVNT.Seek(0x13, SEEK_CUR);
        u32 CharIndex = rEVNT.ReadLong();
        rEVNT.Seek(0xC, SEEK_CUR);
        CAssetID ParticleID = rEVNT.ReadLong();
        mpEventData->AddEvent(CharIndex, ParticleID);

        if (IsEchoes)
            rEVNT.Seek(0x4, SEEK_CUR);
        else
            rEVNT.ReadString();

        rEVNT.Seek(0x8, SEEK_CUR);
    }

    // Sound Events
    if (Version == 2)
    {
        u32 NumSoundEvents = rEVNT.ReadLong();

        for (u32 iSound = 0; iSound < NumSoundEvents; iSound++)
        {
            rEVNT.Seek(0x2, SEEK_CUR);
            rEVNT.ReadString();
            rEVNT.Seek(0x13, SEEK_CUR);
            u32 CharIndex = rEVNT.ReadLong();
            rEVNT.Seek(0x4, SEEK_CUR);
            u32 SoundID = rEVNT.ReadLong() & 0xFFFF;
            rEVNT.Seek(0x8, SEEK_CUR);
            if (IsEchoes) rEVNT.Seek(0xC, SEEK_CUR);

            if (SoundID != 0xFFFF)
            {
                SSoundInfo SoundInfo = mpEventData->Entry()->Project()->AudioManager()->GetSoundInfo(SoundID);

                if (SoundInfo.pAudioGroup)
                    mpEventData->AddEvent(CharIndex, SoundInfo.pAudioGroup->ID());
            }
        }
    }
}

// ************ STATIC ************
CAnimEventData* CAnimEventLoader::LoadEVNT(IInputStream& rEVNT, CResourceEntry *pEntry)
{
    CAnimEventLoader Loader;
    Loader.mpEventData = new CAnimEventData(pEntry);
    Loader.LoadEvents(rEVNT, false);
    return Loader.mpEventData;
}

CAnimEventData* CAnimEventLoader::LoadAnimSetEvents(IInputStream& rANCS)
{
    CAnimEventLoader Loader;
    Loader.mpEventData = new CAnimEventData();
    Loader.LoadEvents(rANCS, true);
    return Loader.mpEventData;
}
