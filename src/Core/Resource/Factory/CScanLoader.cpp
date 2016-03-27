#include "CScanLoader.h"
#include "Core/Resource/CResCache.h"
#include <Common/Log.h>

CScanLoader::CScanLoader()
{
}

CScan* CScanLoader::LoadScanMP1(IInputStream& rSCAN)
{
    // Basic support at the moment - don't read animation/scan image data
    rSCAN.Seek(0x4, SEEK_CUR); // Skip FRME ID
    mpScan->mpStringTable = gResCache.GetResource(rSCAN.ReadLong(), "STRG");
    mpScan->mIsSlow = (rSCAN.ReadLong() != 0);
    mpScan->mCategory = (CScan::ELogbookCategory) rSCAN.ReadLong();
    mpScan->mIsImportant = (rSCAN.ReadByte() == 1);
    mpScan->mVersion = ePrime;
    return mpScan;
}

CScan* CScanLoader::LoadScanMP2(IInputStream& rSCAN)
{
    // The SCAN format in MP2 embeds a SNFO object using the same format as SCLY
    // However since the contents of the file are consistent there's no need to delegate to CScriptLoader
    rSCAN.Seek(0x1, SEEK_CUR);
    u32 NumInstances = rSCAN.ReadLong();

    if (NumInstances != 1) {
        Log::FileError(rSCAN.GetSourceString(), "SCAN has multiple instances");
        return nullptr;
    }

    u32 ScanInfoStart = rSCAN.Tell();

    CFourCC SNFO(rSCAN);
    if (SNFO != "SNFO") {
        Log::FileError(rSCAN.GetSourceString(), ScanInfoStart, "Unrecognized SCAN object type: " + SNFO.ToString());
        return nullptr;
    }

    rSCAN.Seek(0x6, SEEK_CUR);
    u16 NumConnections = rSCAN.ReadShort();
    if (NumConnections > 0) {
        Log::FileWarning(rSCAN.GetSourceString(), ScanInfoStart, "SNFO object in SCAN has connections");
        rSCAN.Seek(NumConnections * 0xC, SEEK_CUR);
    }

    u32 BasePropID = rSCAN.ReadLong();
    if (BasePropID != 0xFFFFFFFF) {
        Log::FileError(rSCAN.GetSourceString(), rSCAN.Tell() - 4, "Invalid base property ID: " + TString::HexString(BasePropID));
        return nullptr;
    }

    rSCAN.Seek(0x2, SEEK_CUR);
    u16 NumProperties = rSCAN.ReadShort();

    switch (NumProperties)
    {
    case 0x14:
    case 0xB:
        mpScan = new CScan();
        LoadParamsMP2(rSCAN);
        break;
    case 0x12:
    case 0x16:
        mpScan = new CScan();
        LoadParamsMP3(rSCAN);
        break;
    default:
        Log::FileError(rSCAN.GetSourceString(), rSCAN.Tell() - 2, "Invalid SNFO property count: " + TString::HexString(NumProperties));
        return nullptr;
    }

    return mpScan;
}

void CScanLoader::LoadParamsMP2(IInputStream& rSCAN)
{
    // Function begins after the SNFO property count
    for (u32 iProp = 0; iProp < 20; iProp++)
    {
        u32 PropertyID = rSCAN.ReadLong();
        u16 PropertySize = rSCAN.ReadShort();
        u32 Next = rSCAN.Tell() + PropertySize;

        switch (PropertyID)
        {
        case 0x2F5B6423:
            mpScan->mpStringTable = gResCache.GetResource(rSCAN.ReadLong(), "STRG");
            break;

        case 0xC308A322:
            mpScan->mIsSlow = (rSCAN.ReadLong() != 0);
            break;

        case 0x7B714814:
            mpScan->mIsImportant = (rSCAN.ReadByte() != 0);
            break;
        }

        rSCAN.Seek(Next, SEEK_SET);
    }

    mpScan->mCategory = CScan::eNone;
    mpScan->mVersion = eEchoes;
}

void CScanLoader::LoadParamsMP3(IInputStream& rSCAN)
{
    // Function begins after the SNFO property count
    // Function is near-identical to the MP2 one, but when I add support
    // for the other params, there will be more differences
    for (u32 iProp = 0; iProp < 20; iProp++)
    {
        u32 PropertyID = rSCAN.ReadLong();
        u16 PropertySize = rSCAN.ReadShort();
        u32 Next = rSCAN.Tell() + PropertySize;

        switch (PropertyID)
        {
        case 0x2F5B6423:
            mpScan->mpStringTable = gResCache.GetResource(rSCAN.ReadLongLong(), "STRG");
            break;

        case 0xC308A322:
            mpScan->mIsSlow = (rSCAN.ReadLong() != 0);
            break;

        case 0x7B714814:
            mpScan->mIsImportant = (rSCAN.ReadByte() != 0);
            break;
        }

        rSCAN.Seek(Next, SEEK_SET);
    }

    mpScan->mCategory = CScan::eNone;
    mpScan->mVersion = eCorruption;
}

// ************ STATIC/PUBLIC ************
CScan* CScanLoader::LoadSCAN(IInputStream& rSCAN)
{
    if (!rSCAN.IsValid()) return nullptr;

    /* Switching to EGame enum here isn't really useful unfortunately
     * because the MP1 demo can be 1, 2, or 3, while MP1 is 5 and MP2+ is 2
     * MP1 is the only one that starts with 5 so that is a consistent check for now
     * Better version checks will be implemented when the other versions are
     * better-understood. */
    u32 FileVersion = rSCAN.ReadLong();
    u32 Magic = rSCAN.ReadLong();

    // Echoes+
    if (CFourCC(FileVersion) == "SCAN")
    {
        // The MP2 load function will check for MP3
        CScanLoader Loader;
        Loader.mVersion = eEchoes;
        if (Magic == 0x01000000) rSCAN.Seek(-4, SEEK_CUR); // The version number isn't present in the Echoes demo
        return Loader.LoadScanMP2(rSCAN);
    }

    if (Magic != 0x0BADBEEF)
    {
        Log::FileError(rSCAN.GetSourceString(), "Invalid SCAN magic: " + TString::HexString(Magic));
        return nullptr;
    }

    if (FileVersion != 5)
    {
        Log::FileError(rSCAN.GetSourceString(), "Unsupported SCAN version: " + TString::HexString(FileVersion, 0));
        return nullptr;
    }

    // MP1 SCAN - read the file!
    CScanLoader Loader;
    Loader.mVersion = ePrime;
    Loader.mpScan = new CScan();
    return Loader.LoadScanMP1(rSCAN);
}
