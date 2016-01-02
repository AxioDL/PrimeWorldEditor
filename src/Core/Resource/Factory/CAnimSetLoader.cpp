#include "CAnimSetLoader.h"
#include "Core/Resource/CResCache.h"
#include "Core/Log.h"

CAnimSetLoader::CAnimSetLoader()
{
}

CAnimSet* CAnimSetLoader::LoadCorruptionCHAR(IInputStream& CHAR)
{
    // For now, we only read enough to fetch the model
    CHAR.Seek(0x1, SEEK_CUR);
    set->nodes.resize(1);
    CAnimSet::SNode& node = set->nodes[0];

    node.name = CHAR.ReadString();
    node.model = gResCache.GetResource(CHAR.ReadLongLong(), "CMDL");
    return set;
}

CAnimSet* CAnimSetLoader::LoadReturnsCHAR(IInputStream& CHAR)
{
    // For now, we only read enough to fetch the model
    CHAR.Seek(0x16, SEEK_CUR);
    set->nodes.resize(1);
    CAnimSet::SNode& node = set->nodes[0];

    node.name = CHAR.ReadString();
    CHAR.Seek(0x14, SEEK_CUR);
    CHAR.ReadString();
    node.model = gResCache.GetResource(CHAR.ReadLongLong(), "CMDL");
    return set;
}

void CAnimSetLoader::LoadPASDatabase(IInputStream& PAS4)
{
    // For now, just parse the data; don't store it
    PAS4.Seek(0x4, SEEK_CUR); // Skipping PAS4 FourCC
    u32 anim_state_count = PAS4.ReadLong();
    PAS4.Seek(0x4, SEEK_CUR); // Skipping default anim state

    for (u32 s = 0; s < anim_state_count; s++)
    {
        PAS4.Seek(0x4, SEEK_CUR); // Skipping unknown value
        u32 parm_info_count = PAS4.ReadLong();
        u32 anim_info_count = PAS4.ReadLong();

        u32 skip = 0;
        for (u32 p = 0; p < parm_info_count; p++)
        {
            u32 type = PAS4.ReadLong();
            PAS4.Seek(0x8, SEEK_CUR);

            switch (type) {
                case 0: // Int32
                case 1: // Uint32
                case 2: // Real32
                case 4: // Enum
                    PAS4.Seek(0x8, SEEK_CUR);
                    skip += 4;
                    break;
                case 3: // Bool
                    PAS4.Seek(0x2, SEEK_CUR);
                    skip++;
                    break;
            }
        }

        for (u32 a = 0; a < anim_info_count; a++)
            PAS4.Seek(0x4 + skip, SEEK_CUR);
    }
}

// ************ STATIC ************
CAnimSet* CAnimSetLoader::LoadANCS(IInputStream& ANCS)
{
    if (!ANCS.IsValid()) return nullptr;
    Log::Write("Loading " + ANCS.GetSourceString());

    u32 magic = ANCS.ReadLong();
    if (magic != 0x00010001)
    {
        Log::FileError(ANCS.GetSourceString(), "Invalid ANCS magic: " + TString::HexString(magic));
        return nullptr;
    }

    CAnimSetLoader loader;
    loader.set = new CAnimSet;

    u32 node_count = ANCS.ReadLong();
    loader.set->nodes.resize(node_count);

    for (u32 n = 0; n < node_count; n++)
    {
        CAnimSet::SNode *node = &loader.set->nodes[n];

        ANCS.Seek(0x4, SEEK_CUR); // Skipping node self-index
        u16 unknown1 = ANCS.ReadShort();
        if (n == 0) loader.mVersion = (unknown1 == 0xA) ? eEchoes : ePrime; // Best version indicator we know of unfortunately
        node->name = ANCS.ReadString();
        node->model = gResCache.GetResource(ANCS.ReadLong(), "CMDL");
        node->skinID = ANCS.ReadLong();
        node->skelID = ANCS.ReadLong();

        // Unfortunately that's all that's actually supported at the moment. Hope to expand later.
        // Since there's no size value I have to actually read the rest of the node to reach the next one
        u32 anim_count = ANCS.ReadLong();
        for (u32 a = 0; a < anim_count; a++)
        {
            ANCS.Seek(0x4, SEEK_CUR);
            if (loader.mVersion == ePrime) ANCS.Seek(0x1, SEEK_CUR);
            ANCS.ReadString();
        }

        // PAS Database
        loader.LoadPASDatabase(ANCS);

        // Particles
        u32 particle_count = ANCS.ReadLong();
        ANCS.Seek(particle_count * 4, SEEK_CUR);
        u32 swoosh_count = ANCS.ReadLong();
        ANCS.Seek(swoosh_count * 4, SEEK_CUR);
        if (unknown1 != 5) ANCS.Seek(0x4, SEEK_CUR);
        u32 electric_count = ANCS.ReadLong();
        ANCS.Seek(electric_count * 4, SEEK_CUR);
        if (loader.mVersion == eEchoes) {
            u32 spsc_count = ANCS.ReadLong();
            ANCS.Seek(spsc_count * 4, SEEK_CUR);
        }
        ANCS.Seek(0x4, SEEK_CUR);
        if (loader.mVersion == eEchoes) ANCS.Seek(0x4, SEEK_CUR);

        u32 anim_count2 = ANCS.ReadLong();
        for (u32 a = 0; a < anim_count2; a++)
        {
            ANCS.ReadString();
            ANCS.Seek(0x18, SEEK_CUR);
        }

        u32 EffectGroupCount = ANCS.ReadLong();
        for (u32 g = 0; g < EffectGroupCount; g++)
        {
            ANCS.ReadString();
            u32 EffectCount = ANCS.ReadLong();

            for (u32 e = 0; e < EffectCount; e++)
            {
                ANCS.ReadString();
                ANCS.Seek(0x8, SEEK_CUR);
                if (loader.mVersion == ePrime) ANCS.ReadString();
                if (loader.mVersion == eEchoes) ANCS.Seek(0x4, SEEK_CUR);
                ANCS.Seek(0xC, SEEK_CUR);
            }
        }
        ANCS.Seek(0x8, SEEK_CUR);

        u32 unknown_count = ANCS.ReadLong();
        ANCS.Seek(unknown_count * 4, SEEK_CUR);

        if (loader.mVersion == eEchoes)
        {
            ANCS.Seek(0x5, SEEK_CUR);
            u32 unknown_count2 = ANCS.ReadLong();
            ANCS.Seek(unknown_count2 * 0x1C, SEEK_CUR);
        }
        // Lots of work for data I'm not even using x.x
    }

    return loader.set;
}

CAnimSet* CAnimSetLoader::LoadCHAR(IInputStream &CHAR)
{
    if (!CHAR.IsValid()) return nullptr;
    Log::Write("Loading " + CHAR.GetSourceString());

    CAnimSetLoader loader;
    u8 check = CHAR.ReadByte();

    if (check == 0x5 || check == 0x3)
    {
        loader.mVersion = eCorruption;
        loader.set = new CAnimSet();
        return loader.LoadCorruptionCHAR(CHAR);
    }

    if (check == 0x59)
    {
        loader.mVersion = eReturns;
        loader.set = new CAnimSet();
        return loader.LoadReturnsCHAR(CHAR);
    }

    Log::FileError(CHAR.GetSourceString(), "CHAR has invalid first byte: " + TString::HexString(check));
    return nullptr;
}
