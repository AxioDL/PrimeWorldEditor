#include "CStringLoader.h"
#include "Core/Log.h"

CStringLoader::CStringLoader()
{
}

void CStringLoader::LoadPrimeDemoSTRG(CInputStream& STRG)
{
    // This function starts at 0x4 in the file - right after the size
    // This STRG version only supports one language per file
    mpStringTable->mLangTables.resize(1);
    CStringTable::SLangTable* Lang = &mpStringTable->mLangTables[1];
    Lang->Language = "ENGL";
    u32 TableStart = STRG.Tell();

    // Header
    u32 NumStrings = STRG.ReadLong();
    Lang->Strings.resize(NumStrings);
    mpStringTable->mNumStrings = NumStrings;

    // String offsets (yeah, that wasn't much of a header)
    std::vector<u32> StringOffsets(NumStrings);
    for (u32 iOff = 0; iOff < StringOffsets.size(); iOff++)
        StringOffsets[iOff] = STRG.ReadLong();

    // Strings
    for (u32 iStr = 0; iStr < NumStrings; iStr++)
    {
        STRG.Seek(TableStart + StringOffsets[iStr], SEEK_SET);
        Lang->Strings[iStr] = STRG.ReadWString();
    }
}

void CStringLoader::LoadPrimeSTRG(CInputStream& STRG)
{
    // This function starts at 0x8 in the file, after magic/version
    // Header
    u32 NumLanguages = STRG.ReadLong();
    u32 NumStrings = STRG.ReadLong();
    mpStringTable->mNumStrings = NumStrings;

    // Language definitions
    mpStringTable->mLangTables.resize(NumLanguages);
    std::vector<u32> LangOffsets(NumLanguages);

    for (u32 iLang = 0; iLang < NumLanguages; iLang++)
    {
        mpStringTable->mLangTables[iLang].Language = CFourCC(STRG);
        LangOffsets[iLang] = STRG.ReadLong();
        if (mVersion == eEchoes) STRG.Seek(0x4, SEEK_CUR); // Skipping strings size
    }

    // String names
    if (mVersion == eEchoes)
        LoadNameTable(STRG);

    // Strings
    u32 StringsStart = STRG.Tell();
    for (u32 iLang = 0; iLang < NumLanguages; iLang++)
    {
        STRG.Seek(StringsStart + LangOffsets[iLang], SEEK_SET);
        if (mVersion == ePrime) STRG.Seek(0x4, SEEK_CUR); // Skipping strings size

        u32 LangStart = STRG.Tell();
        CStringTable::SLangTable* pLang = &mpStringTable->mLangTables[iLang];
        pLang->Strings.resize(NumStrings);

        // Offsets
        std::vector<u32> StringOffsets(NumStrings);
        for (u32 iOff = 0; iOff < NumStrings; iOff++)
            StringOffsets[iOff] = STRG.ReadLong();

        // The actual strings
        for (u32 iStr = 0; iStr < NumStrings; iStr++)
        {
            STRG.Seek(LangStart + StringOffsets[iStr], SEEK_SET);
            pLang->Strings[iStr] = STRG.ReadWString();
        }
    }
}

void CStringLoader::LoadCorruptionSTRG(CInputStream& STRG)
{
    // This function starts at 0x8 in the file, after magic/version
    // Header
    u32 NumLanguages = STRG.ReadLong();
    u32 NumStrings = STRG.ReadLong();
    mpStringTable->mNumStrings = NumStrings;

    // String names
    LoadNameTable(STRG);

    // Language definitions
    mpStringTable->mLangTables.resize(NumLanguages);
    std::vector<std::vector<u32>> LangOffsets(NumLanguages);

    for (u32 iLang = 0; iLang < NumLanguages; iLang++)
        mpStringTable->mLangTables[iLang].Language = CFourCC(STRG);

    for (u32 iLang = 0; iLang < NumLanguages; iLang++)
    {
        LangOffsets[iLang].resize(NumStrings);

        STRG.Seek(0x4, SEEK_CUR); // Skipping total string size

        for (u32 iStr = 0; iStr < NumStrings; iStr++)
            LangOffsets[iLang][iStr] = STRG.ReadLong();
    }

    // Strings
    u32 StringsStart = STRG.Tell();

    for (u32 iLang = 0; iLang < NumLanguages; iLang++)
    {
        CStringTable::SLangTable *pLang = &mpStringTable->mLangTables[iLang];
        pLang->Strings.resize(NumStrings);

        for (u32 iStr = 0; iStr < NumStrings; iStr++)
        {
            STRG.Seek(StringsStart + LangOffsets[iLang][iStr], SEEK_SET);
            STRG.Seek(0x4, SEEK_CUR); // Skipping string size

            pLang->Strings[iStr] = STRG.ReadString();
        }
    }
}

void CStringLoader::LoadNameTable(CInputStream& STRG)
{
    // Name table header
    u32 NameCount = STRG.ReadLong();
    u32 NameTableSize = STRG.ReadLong();
    u32 NameTableStart = STRG.Tell();
    u32 NameTableEnd = NameTableStart + NameTableSize;

    // Name definitions
    struct SNameDef {
        u32 NameOffset, StringIndex;
    };
    std::vector<SNameDef> NameDefs(NameCount);

    for (u32 iName = 0; iName < NameCount; iName++)
    {
        NameDefs[iName].NameOffset = STRG.ReadLong() + NameTableStart;
        NameDefs[iName].StringIndex = STRG.ReadLong();
    }

    // Name strings
    mpStringTable->mStringNames.resize(mpStringTable->mNumStrings);
    for (u32 iName = 0; iName < NameCount; iName++)
    {
        SNameDef *pDef = &NameDefs[iName];
        STRG.Seek(pDef->NameOffset, SEEK_SET);
        mpStringTable->mStringNames[pDef->StringIndex] = STRG.ReadString();
    }
    STRG.Seek(NameTableEnd, SEEK_SET);
}

// ************ STATIC ************
CStringTable* CStringLoader::LoadSTRG(CInputStream& STRG)
{
    // Verify that this is a valid STRG
    if (!STRG.IsValid()) return nullptr;
    Log::Write("Loading " + STRG.GetSourceString());

    u32 Magic = STRG.ReadLong();
    EGame Version = eUnknownVersion;

    if (Magic != 0x87654321)
    {
        // Check for MP1 Demo STRG format - no magic/version; the first value is actually the filesize
        // so the best I can do is verify the first value actually points to the end of the file
        if (Magic <= (u32) STRG.Size())
        {
            STRG.Seek(Magic, SEEK_SET);
            if ((STRG.EoF()) || (STRG.ReadShort() == 0xFFFF))
                Version = ePrimeDemo;
        }

        if (Version != ePrimeDemo)
        {
            Log::FileError(STRG.GetSourceString(), "Invalid STRG magic: " + TString::HexString(Magic));
            return nullptr;
        }
    }

    else
    {
        u32 FileVersion = STRG.ReadLong();
        Version = GetFormatVersion(FileVersion);

        if (FileVersion == eUnknownVersion)
        {
            Log::FileError(STRG.GetSourceString(), "Unsupported STRG version: " + TString::HexString(FileVersion));
            return nullptr;
        }
    }

    // Valid; now we create the loader and call the function that reads the rest of the file
    CStringLoader Loader;
    Loader.mpStringTable = new CStringTable();
    Loader.mVersion = Version;

    if (Version == ePrimeDemo) Loader.LoadPrimeDemoSTRG(STRG);
    else if (Version < eCorruption) Loader.LoadPrimeSTRG(STRG);
    else Loader.LoadCorruptionSTRG(STRG);

    return Loader.mpStringTable;
}

EGame CStringLoader::GetFormatVersion(u32 Version)
{
    switch (Version)
    {
    case 0x0: return ePrime;
    case 0x1: return eEchoes;
    case 0x3: return eCorruption;
    default: return eUnknownVersion;
    }
}
