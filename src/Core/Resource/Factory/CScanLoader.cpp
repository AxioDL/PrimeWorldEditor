#include "CScanLoader.h"
#include "Core/GameProject/CResourceStore.h"
#include <Common/Log.h>

CScanLoader::CScanLoader()
{
}

CScan* CScanLoader::LoadScanMP1(IInputStream& rSCAN)
{
    // Basic support at the moment - don't read animation/scan image data
    mpScan->mFrameID = CAssetID(rSCAN, e32Bit);
    mpScan->mpStringTable = gpResourceStore->LoadResource(rSCAN.ReadLong(), eStringTable);
    mpScan->mIsSlow = (rSCAN.ReadLong() != 0);
    mpScan->mCategory = (CScan::ELogbookCategory) rSCAN.ReadLong();
    mpScan->mIsImportant = (rSCAN.ReadByte() == 1);

    for (uint32 iImg = 0; iImg < 4; iImg++)
    {
        mpScan->mScanImageTextures[iImg] = CAssetID(rSCAN, e32Bit);
        rSCAN.Seek(0x18, SEEK_CUR);
    }

    return mpScan;
}

CScan* CScanLoader::LoadScanMP2(IInputStream& rSCAN)
{
    // The SCAN format in MP2 embeds a SNFO object using the same format as SCLY
    // However since the contents of the file are consistent there's no need to delegate to CScriptLoader
    rSCAN.Skip(0x1);
    uint32 NumInstances = rSCAN.ReadLong();

    if (NumInstances != 1)
    {
        errorf("%s: SCAN has multiple instances", *rSCAN.GetSourceString());
        return nullptr;
    }

    uint32 ScanInfoStart = rSCAN.Tell();

    CFourCC SNFO(rSCAN);
    if (SNFO != FOURCC('SNFO'))
    {
        errorf("%s [0x%X]: Unrecognized SCAN object type: %s", *rSCAN.GetSourceString(), ScanInfoStart, *SNFO.ToString());
        return nullptr;
    }

    uint16 InstanceSize = rSCAN.ReadShort();
    uint32 InstanceEnd = rSCAN.Tell() + InstanceSize;
    rSCAN.Skip(0x4);

    uint16 NumConnections = rSCAN.ReadShort();
    if (NumConnections > 0)
    {
        warnf("%s [0x%X]: SNFO object in SCAN has connections", *rSCAN.GetSourceString(), ScanInfoStart);
        rSCAN.Skip(NumConnections * 0xC);
    }

    uint32 BasePropID = rSCAN.ReadLong();
    if (BasePropID != 0xFFFFFFFF)
    {
        errorf("%s [0x%X]: Invalid base property ID: 0x%08X", *rSCAN.GetSourceString(), rSCAN.Tell() - 4, BasePropID);
        return nullptr;
    }

    rSCAN.Skip(0x2);
    uint16 NumProperties = rSCAN.ReadShort();

    switch (NumProperties)
    {
    case 0x14:
    case 0xB:
        mpScan = new CScan(mpEntry);
        LoadParamsMP2(rSCAN, NumProperties);
        break;
    case 0x12:
    case 0x15:
    case 0x16:
        mpScan = new CScan(mpEntry);
        LoadParamsMP3(rSCAN, NumProperties);
        break;
    default:
        errorf("%s [0x%X]: Invalid SNFO property count: 0x%X", *rSCAN.GetSourceString(), rSCAN.Tell() - 2, NumProperties);
        return nullptr;
    }

    // Load MP3 dependency list
    if (mpScan->Game() == EGame::Corruption)
    {
        rSCAN.GoTo(InstanceEnd);
        uint32 NumDeps = rSCAN.ReadLong();

        for (uint32 DepIdx = 0; DepIdx < NumDeps; DepIdx++)
        {
            rSCAN.Skip(4);
            CAssetID ID(rSCAN, mpScan->Game());
            mpScan->mDependencyList.push_back(ID);
        }
    }

    return mpScan;
}

void CScanLoader::LoadParamsMP2(IInputStream& rSCAN, uint16 NumProperties)
{
    // Function begins after the SNFO property count
    mpScan->mSecondaryModels.resize(9);

    for (uint32 iProp = 0; iProp < NumProperties; iProp++)
    {
        uint32 PropertyID = rSCAN.ReadLong();
        uint16 PropertySize = rSCAN.ReadShort();
        uint32 Next = rSCAN.Tell() + PropertySize;

        switch (PropertyID)
        {
        case 0x2F5B6423:
            mpScan->mpStringTable = gpResourceStore->LoadResource(rSCAN.ReadLong(), eStringTable);
            break;

        case 0xC308A322:
            mpScan->mIsSlow = (rSCAN.ReadLong() != 0);
            break;

        case 0x7B714814:
            mpScan->mIsImportant = rSCAN.ReadBool();
            break;

        case 0x1733B1EC:
            mpScan->mUseLogbookModelPostScan = rSCAN.ReadBool();
            break;

        case 0x53336141:
            mpScan->mPostOverrideTexture = CAssetID(rSCAN, mVersion);
            break;

        case 0x3DE0BA64:
            mpScan->mLogbookDefaultRotX = rSCAN.ReadFloat();
            break;

        case 0x2ADD6628:
            mpScan->mLogbookDefaultRotZ = rSCAN.ReadFloat();
            break;

        case 0xD0C15066:
            mpScan->mLogbookScale = rSCAN.ReadFloat();
            break;

        case 0xB7ADC418:
            mpScan->mLogbookModel = CAssetID(rSCAN, mVersion);
            break;

        case 0x15694EE1:
            mpScan->mLogbookAnimParams = CAnimationParameters(rSCAN, mVersion);
            break;

        case 0x58F9FE99:
            mpScan->mUnknownAnimParams = CAnimationParameters(rSCAN, mVersion);
            break;

        case 0x1C5B4A3A:
            LoadScanInfoSecondaryModel( rSCAN, mpScan->mSecondaryModels[0] );
            break;

        case 0x8728A0EE:
            LoadScanInfoSecondaryModel( rSCAN, mpScan->mSecondaryModels[1] );
            break;

        case 0xF1CD99D3:
            LoadScanInfoSecondaryModel( rSCAN, mpScan->mSecondaryModels[2] );
            break;

        case 0x6ABE7307:
            LoadScanInfoSecondaryModel( rSCAN, mpScan->mSecondaryModels[3] );
            break;

        case 0x1C07EBA9:
            LoadScanInfoSecondaryModel( rSCAN, mpScan->mSecondaryModels[4] );
            break;

        case 0x8774017D:
            LoadScanInfoSecondaryModel( rSCAN, mpScan->mSecondaryModels[5] );
            break;

        case 0xF1913840:
            LoadScanInfoSecondaryModel( rSCAN, mpScan->mSecondaryModels[6] );
            break;

        case 0x6AE2D294:
            LoadScanInfoSecondaryModel( rSCAN, mpScan->mSecondaryModels[7] );
            break;

        case 0x1CE2091C:
            LoadScanInfoSecondaryModel( rSCAN, mpScan->mSecondaryModels[8] );
            break;
        }

        rSCAN.GoTo(Next);
    }

    mpScan->mCategory = CScan::eNone;
}

void CScanLoader::LoadParamsMP3(IInputStream& rSCAN, uint16 NumProperties)
{
    // Function begins after the SNFO property count
    for (uint32 iProp = 0; iProp < NumProperties; iProp++)
    {
        uint32 PropertyID = rSCAN.ReadLong();
        uint16 PropertySize = rSCAN.ReadShort();
        uint32 Next = rSCAN.Tell() + PropertySize;

        switch (PropertyID)
        {
        case 0x2F5B6423:
            mpScan->mpStringTable = gpResourceStore->LoadResource(rSCAN.ReadLongLong(), eStringTable);
            break;

        case 0xC308A322:
            mpScan->mIsSlow = (rSCAN.ReadLong() != 0);
            break;

        case 0x7B714814:
            mpScan->mIsImportant = (rSCAN.ReadByte() != 0);
            break;
        }

        rSCAN.GoTo(Next);
    }

    mpScan->mCategory = CScan::eNone;
}

void CScanLoader::LoadScanInfoSecondaryModel(IInputStream& rSCAN, CScan::SScanInfoSecondaryModel& rSecondaryModel)
{
    uint16 NumProperties = rSCAN.ReadShort();

    for (uint32 iProp = 0; iProp < NumProperties; iProp++)
    {
        uint32 PropertyID = rSCAN.ReadLong();
        uint16 PropertySize = rSCAN.ReadShort();
        uint32 Next = rSCAN.Tell() + PropertySize;

        switch (PropertyID)
        {
        case 0x1F7921BC:
            rSecondaryModel.ModelID = CAssetID(rSCAN, mVersion);
            break;

        case 0xCDD202D1:
            rSecondaryModel.AnimParams = CAnimationParameters(rSCAN, mVersion);
            break;

        case 0x3EA2BED8:
            rSecondaryModel.AttachBoneName = rSCAN.ReadString();
            break;
        }

        rSCAN.GoTo(Next);
    }
}

// ************ STATIC/PUBLIC ************
CScan* CScanLoader::LoadSCAN(IInputStream& rSCAN, CResourceEntry *pEntry)
{
    if (!rSCAN.IsValid()) return nullptr;

    /* Switching to EGame enum here isn't really useful unfortunately
     * because the MP1 demo can be 1, 2, or 3, while MP1 is 5 and MP2+ is 2
     * MP1 is the only one that starts with 5 so that is a consistent check for now
     * Better version checks will be implemented when the other versions are
     * better-understood. */
    uint32 FileVersion = rSCAN.ReadLong();
    uint32 Magic = rSCAN.ReadLong();

    // Echoes+
    if (FileVersion == FOURCC('SCAN'))
    {
        // The MP2 load function will check for MP3
        CScanLoader Loader;
        Loader.mVersion = EGame::Echoes;
        Loader.mpEntry = pEntry;
        if (Magic == 0x01000000) rSCAN.Seek(-4, SEEK_CUR); // The version number isn't present in the Echoes demo
        return Loader.LoadScanMP2(rSCAN);
    }

    if (Magic != 0x0BADBEEF)
    {
        errorf("%s: Invalid SCAN magic: 0x%08X", *rSCAN.GetSourceString(), Magic);
        return nullptr;
    }

    if (FileVersion != 5)
    {
        errorf("%s: Unsupported SCAN version: 0x%X", *rSCAN.GetSourceString(), FileVersion);
        return nullptr;
    }

    // MP1 SCAN - read the file!
    CScanLoader Loader;
    Loader.mVersion = EGame::Prime;
    Loader.mpScan = new CScan(pEntry);
    Loader.mpEntry = pEntry;
    return Loader.LoadScanMP1(rSCAN);
}
