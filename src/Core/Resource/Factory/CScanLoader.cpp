#include "CScanLoader.h"
#include "Core/Resource/CResCache.h"
#include "Core/Log.h"

CScanLoader::CScanLoader()
{
}

CScan* CScanLoader::LoadScanMP1(IInputStream &SCAN)
{
    // Basic support at the moment - don't read animation/scan image data
    SCAN.Seek(0x4, SEEK_CUR); // Skip FRME ID
    mpScan->mpStringTable = gResCache.GetResource(SCAN.ReadLong(), "STRG");
    mpScan->mIsSlow = (SCAN.ReadLong() != 0);
    mpScan->mCategory = (CScan::ELogbookCategory) SCAN.ReadLong();
    mpScan->mIsImportant = (SCAN.ReadByte() == 1);
    mpScan->mVersion = ePrime;
    return mpScan;
}

CScan* CScanLoader::LoadScanMP2(IInputStream& SCAN)
{
    // The SCAN format in MP2 embeds a SNFO object using the same format as SCLY
    // However since the contents of the file are consistent there's no need to delegate to CScriptLoader
    SCAN.Seek(0x1, SEEK_CUR);
    u32 NumInstances = SCAN.ReadLong();

    if (NumInstances != 1) {
        Log::FileError(SCAN.GetSourceString(), "SCAN has multiple instances");
        return nullptr;
    }

    u32 ScanInfoStart = SCAN.Tell();

    CFourCC SNFO(SCAN);
    if (SNFO != "SNFO") {
        Log::FileError(SCAN.GetSourceString(), ScanInfoStart, "Unrecognized SCAN object type: " + SNFO.ToString());
        return nullptr;
    }

    SCAN.Seek(0x6, SEEK_CUR);
    u16 NumConnections = SCAN.ReadShort();
    if (NumConnections > 0) {
        Log::FileWarning(SCAN.GetSourceString(), ScanInfoStart, "SNFO object in SCAN has connections");
        SCAN.Seek(NumConnections * 0xC, SEEK_CUR);
    }

    u32 BasePropID = SCAN.ReadLong();
    if (BasePropID != 0xFFFFFFFF) {
        Log::FileError(SCAN.GetSourceString(), SCAN.Tell() - 4, "Invalid base proprty ID: " + TString::HexString(BasePropID));
        return nullptr;
    }

    SCAN.Seek(0x2, SEEK_CUR);
    u16 NumProperties = SCAN.ReadShort();

    switch (NumProperties)
    {
    case 0x14:
    case 0xB:
        mpScan = new CScan();
        LoadParamsMP2(SCAN);
        break;
    case 0x12:
    case 0x16:
        mpScan = new CScan();
        LoadParamsMP3(SCAN);
        break;
    default:
        Log::FileError(SCAN.GetSourceString(), SCAN.Tell() - 2, "Invalid SNFO property count: " + TString::HexString(NumProperties));
        return nullptr;
    }

    return mpScan;
}

void CScanLoader::LoadParamsMP2(IInputStream& SCAN)
{
    // Function begins after the SNFO property count
    for (u32 iProp = 0; iProp < 20; iProp++)
    {
        u32 PropertyID = SCAN.ReadLong();
        u16 PropertySize = SCAN.ReadShort();
        u32 Next = SCAN.Tell() + PropertySize;

        switch (PropertyID)
        {
        case 0x2F5B6423:
            mpScan->mpStringTable = gResCache.GetResource(SCAN.ReadLong(), "STRG");
            break;

        case 0xC308A322:
            mpScan->mIsSlow = (SCAN.ReadLong() != 0);
            break;

        case 0x7B714814:
            mpScan->mIsImportant = (SCAN.ReadByte() != 0);
            break;
        }

        SCAN.Seek(Next, SEEK_SET);
    }

    mpScan->mCategory = CScan::eNone;
    mpScan->mVersion = eEchoes;
}

void CScanLoader::LoadParamsMP3(IInputStream& SCAN)
{
    // Function begins after the SNFO property count
    // Function is near-identical to the MP2 one, but when I add support
    // for the other params, there will be more differences
    for (u32 iProp = 0; iProp < 20; iProp++)
    {
        u32 PropertyID = SCAN.ReadLong();
        u16 PropertySize = SCAN.ReadShort();
        u32 Next = SCAN.Tell() + PropertySize;

        switch (PropertyID)
        {
        case 0x2F5B6423:
            mpScan->mpStringTable = gResCache.GetResource(SCAN.ReadLongLong(), "STRG");
            break;

        case 0xC308A322:
            mpScan->mIsSlow = (SCAN.ReadLong() != 0);
            break;

        case 0x7B714814:
            mpScan->mIsImportant = (SCAN.ReadByte() != 0);
            break;
        }

        SCAN.Seek(Next, SEEK_SET);
    }

    mpScan->mCategory = CScan::eNone;
    mpScan->mVersion = eCorruption;
}

// ************ STATIC/PUBLIC ************
CScan* CScanLoader::LoadSCAN(IInputStream &SCAN)
{
    if (!SCAN.IsValid()) return nullptr;
    Log::Write("Loading " + SCAN.GetSourceString());

    /* Switching to EGame enum here isn't really useful unfortunately
     * because the MP1 demo can be 1, 2, or 3, while MP1 is 5 and MP2+ is 2
     * MP1 is the only one that starts with 5 so that is a consistent check for now
     * Better version checks will be implemented when the other versions are
     * better-understood. */
    u32 fileVersion = SCAN.ReadLong();
    u32 magic = SCAN.ReadLong();

    // Echoes+
    if (CFourCC(fileVersion) == "SCAN")
    {
        // The MP2 load function will check for MP3
        CScanLoader loader;
        loader.mVersion = eEchoes;
        if (magic == 0x01000000) SCAN.Seek(-4, SEEK_CUR); // The version number isn't present in the Echoes demo
        return loader.LoadScanMP2(SCAN);
    }

    if (magic != 0x0BADBEEF)
    {
        Log::FileError(SCAN.GetSourceString(), "Invalid SCAN magic: " + TString::HexString(magic));
        return nullptr;
    }

    if (fileVersion != 5)
    {
        Log::FileError(SCAN.GetSourceString(), "Unsupported SCAN version: " + TString::HexString(fileVersion));
        return nullptr;
    }

    // MP1 SCAN - read the file!
    CScanLoader loader;
    loader.mVersion = ePrime;
    loader.mpScan = new CScan();
    return loader.LoadScanMP1(SCAN);
}
