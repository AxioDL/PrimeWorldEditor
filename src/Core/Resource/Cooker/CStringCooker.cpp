#include "CStringCooker.h"
#include <Common/NBasics.h>
#include <algorithm>

void CStringCooker::WritePrimeDemoSTRG(IOutputStream& STRG)
{
    uint StartOffset = STRG.Tell();
    uint NumStrings = mpStringTable->NumStrings();

    // Start writing the file...
    STRG.WriteLong(0); // Dummy file size
    uint TableStart = STRG.Tell();
    STRG.WriteLong(NumStrings);

    // Dummy string offsets
    for (uint StringIdx = 0; StringIdx < NumStrings; StringIdx++)
        STRG.WriteLong(0);

    // Write strings
    std::vector<uint> StringOffsets(NumStrings);

    for (uint StringIdx = 0; StringIdx < NumStrings; StringIdx++)
    {
        StringOffsets[StringIdx] = STRG.Tell() - TableStart;
        STRG.Write16String( mpStringTable->GetString(ELanguage::English, StringIdx).ToUTF16() );
    }

    // Fill in offsets
    uint FileSize = STRG.Tell() - StartOffset;
    STRG.GoTo(StartOffset);
    STRG.WriteLong(FileSize);
    STRG.Skip(4);

    for (uint StringIdx = 0; StringIdx < NumStrings; StringIdx++)
        STRG.WriteLong( StringOffsets[StringIdx] );
}

void CStringCooker::WritePrimeSTRG(IOutputStream& STRG)
{
    // Magic/Version
    STRG.WriteLong( 0x87654321 );
    STRG.WriteLong( mpStringTable->Game() >= EGame::EchoesDemo ? 1 : 0 );
    STRG.WriteLong( mpStringTable->NumLanguages() );
    STRG.WriteLong( mpStringTable->NumStrings() );

    // Language info
    uint LanguagesStart = STRG.Tell();

    for (uint LanguageIdx = 0; LanguageIdx < mpStringTable->NumLanguages(); LanguageIdx++)
    {
        const CStringTable::SLanguageData& kLanguage = mpStringTable->mLanguages[LanguageIdx];
        STRG.WriteLong( (uint) kLanguage.Language );
        STRG.WriteLong( 0 ); // Dummy offset

        if ( mpStringTable->Game() >= EGame::EchoesDemo )
        {
            STRG.WriteLong( 0 ); // Dummy size
        }
    }

    // Name Table
    if ( mpStringTable->Game() >= EGame::EchoesDemo )
    {
        WriteNameTable(STRG);
    }

    // Strings
    uint StringDataStart = STRG.Tell();
    std::vector<uint> LanguageOffsets( mpStringTable->NumLanguages() );
    std::vector<uint> LanguageSizes( mpStringTable->NumLanguages() );

    for (uint LanguageIdx = 0; LanguageIdx < mpStringTable->NumLanguages(); LanguageIdx++)
    {
        const CStringTable::SLanguageData& kLanguage = mpStringTable->mLanguages[LanguageIdx];

        uint LanguageStart = STRG.Tell();
        LanguageOffsets[LanguageIdx] = LanguageStart - StringDataStart;

        if ( mpStringTable->Game() == EGame::Prime )
        {
            STRG.WriteLong( 0 ); // Dummy size
        }

        // Fill dummy string offsets
        uint StringOffsetBase = STRG.Tell();

        for (uint StringIdx = 0; StringIdx < mpStringTable->NumStrings(); StringIdx++)
        {
            STRG.WriteLong( 0 );
        }

        // Write strings
        std::vector<uint> StringOffsets( mpStringTable->NumStrings() );

        for (uint StringIdx = 0; StringIdx < mpStringTable->NumStrings(); StringIdx++)
        {
            StringOffsets[StringIdx] = STRG.Tell() - StringOffsetBase;
            STRG.Write16String( kLanguage.Strings[StringIdx].String.ToUTF16() );
        }

        // Go back and fill in size/offsets
        uint LanguageEnd = STRG.Tell();
        LanguageSizes[LanguageIdx] = LanguageEnd - StringOffsetBase;
        STRG.GoTo(LanguageStart);

        if ( mpStringTable->Game() == EGame::Prime )
        {
            STRG.WriteLong( LanguageSizes[LanguageIdx] );
        }

        for (uint StringIdx = 0; StringIdx < mpStringTable->NumStrings(); StringIdx++)
        {
            STRG.WriteLong( StringOffsets[StringIdx] );
        }

        STRG.GoTo(LanguageEnd);
    }

    uint STRGEnd = STRG.Tell();

    // Fill in missing language data
    STRG.GoTo(LanguagesStart);

    for (uint LanguageIdx = 0; LanguageIdx < mpStringTable->NumLanguages(); LanguageIdx++)
    {
        STRG.Skip(4); // Skip language ID
        STRG.WriteLong( LanguageOffsets[LanguageIdx] );

        if ( mpStringTable->Game() >= EGame::EchoesDemo )
        {
            STRG.WriteLong( LanguageSizes[LanguageIdx] );
        }
    }

    STRG.GoTo(STRGEnd);
}

void CStringCooker::WriteCorruptionSTRG(IOutputStream& STRG)
{
    // Magic/Version
    STRG.WriteLong( 0x87654321 );
    STRG.WriteLong( 3 );
    STRG.WriteLong( mpStringTable->NumLanguages() );
    STRG.WriteLong( mpStringTable->NumStrings() );

    // Name Table
    WriteNameTable(STRG);

    // Create some structures before continuing...
    // In MP3 and DKCR, if a string has not been localized, then the English text
    // is reused, instead of duplicating the string data like MP1 and MP2 would have.
    struct SCookedLanguageData
    {
        ELanguage Language;
        std::vector<uint> StringOffsets;
        uint TotalSize;
    };
    std::vector<SCookedLanguageData> CookedLanguageData( mpStringTable->NumLanguages() );

    for (uint LanguageIdx = 0; LanguageIdx < mpStringTable->NumLanguages(); LanguageIdx++)
    {
        const CStringTable::SLanguageData& kLanguageData = mpStringTable->mLanguages[LanguageIdx];

        SCookedLanguageData& CookedData = CookedLanguageData[LanguageIdx];
        CookedData.Language = kLanguageData.Language;
        CookedData.StringOffsets.resize( mpStringTable->NumStrings() );
        CookedData.TotalSize = 0;
    }

    // Language IDs
    for (uint LanguageIdx = 0; LanguageIdx < mpStringTable->NumLanguages(); LanguageIdx++)
    {
        STRG.WriteLong( (uint) CookedLanguageData[LanguageIdx].Language );
    }

    // Language Info
    uint LanguageInfoStart = STRG.Tell();

    for (uint LanguageIdx = 0; LanguageIdx < mpStringTable->NumLanguages(); LanguageIdx++)
    {
        // Fill language size/offsets with dummy data...
        STRG.WriteLong( 0 );

        for (uint StringIdx = 0; StringIdx < mpStringTable->NumStrings(); StringIdx++)
            STRG.WriteLong( 0 );
    }

    // Some of the following code assumes that language 0 is English.
    ASSERT( mpStringTable->mLanguages[0].Language == ELanguage::English );

    // Strings
    uint StringsStart = STRG.Tell();

    for (uint StringIdx = 0; StringIdx < mpStringTable->NumStrings(); StringIdx++)
    {
        for (uint LanguageIdx = 0; LanguageIdx < mpStringTable->NumLanguages(); LanguageIdx++)
        {
            const CStringTable::SLanguageData& kLanguageData = mpStringTable->mLanguages[LanguageIdx];
            const CStringTable::SStringData& kStringData = kLanguageData.Strings[StringIdx];
            SCookedLanguageData& CookedData = CookedLanguageData[LanguageIdx];

            // If the "localized" flag is disabled, then we will not write this string. Instead, it will
            // reuse the offset for the English text.
            if (LanguageIdx == 0 || kStringData.IsLocalized)
            {
                CookedData.StringOffsets[StringIdx] = STRG.Tell() - StringsStart;
                CookedData.TotalSize += kStringData.String.Size() + 1; // +1 for terminating zero
                STRG.WriteLong( kStringData.String.Size() + 1 );
                STRG.WriteString( kStringData.String );
            }
            else
            {
                CookedData.StringOffsets[StringIdx] = CookedLanguageData[0].StringOffsets[StringIdx];
                CookedData.TotalSize += mpStringTable->mLanguages[0].Strings[StringIdx].String.Size() + 1; // +1 for terminating zero
            }
        }
    }

    uint STRGEnd = STRG.Tell();

    // Fill in missing language data
    STRG.GoTo(LanguageInfoStart);

    for (uint LanguageIdx = 0; LanguageIdx < mpStringTable->NumLanguages(); LanguageIdx++)
    {
        const SCookedLanguageData& kCookedData = CookedLanguageData[LanguageIdx];
        STRG.WriteLong( kCookedData.TotalSize );

        for (uint StringIdx = 0; StringIdx < mpStringTable->NumStrings(); StringIdx++)
            STRG.WriteLong( kCookedData.StringOffsets[StringIdx] );
    }

    STRG.GoTo(STRGEnd);
}

void CStringCooker::WriteNameTable(IOutputStream& STRG)
{
    // Build a list of name entries to put in the map
    struct SNameEntry
    {
        uint Offset;
        uint Index;
        TString Name;
    };
    std::vector<SNameEntry> NameEntries;

    for (uint StringIdx = 0; StringIdx < mpStringTable->NumStrings(); StringIdx++)
    {
        SNameEntry Entry;
        Entry.Offset = 0;
        Entry.Index = StringIdx;
        Entry.Name = mpStringTable->StringNameByIndex(StringIdx);

        if (!Entry.Name.IsEmpty())
        {
            NameEntries.push_back(Entry);
        }
    }

    std::stable_sort( NameEntries.begin(), NameEntries.end(), [](const SNameEntry& kLHS, const SNameEntry& kRHS) -> bool {
        return kLHS.Name < kRHS.Name;
    });

    // Write out name entries
    uint NameTableStart = STRG.Tell();
    STRG.WriteLong( NameEntries.size() );
    STRG.WriteLong( 0 ); // Dummy name table size
    uint NameTableOffsetsStart = STRG.Tell();

    for (uint NameIdx = 0; NameIdx < NameEntries.size(); NameIdx++)
    {
        STRG.WriteLong( 0 ); // Dummy name offset
        STRG.WriteLong( NameEntries[NameIdx].Index );
    }

    // Write out names
    std::vector<uint> NameOffsets( NameEntries.size() );

    for (uint NameIdx = 0; NameIdx < NameEntries.size(); NameIdx++)
    {
        NameOffsets[NameIdx] = STRG.Tell() - NameTableOffsetsStart;
        STRG.WriteString( NameEntries[NameIdx].Name );
    }

    // Fill out sizes and offsets
    uint NameTableEnd = STRG.Tell();
    uint NameTableSize = NameTableEnd - NameTableOffsetsStart;

    STRG.GoTo(NameTableStart);
    STRG.Skip(4);
    STRG.WriteLong(NameTableSize);

    for (uint NameIdx = 0; NameIdx < NameEntries.size(); NameIdx++)
    {
        STRG.WriteLong( NameOffsets[NameIdx] );
        STRG.Skip(4);
    }

    STRG.GoTo(NameTableEnd);
}

/** Static entry point */
bool CStringCooker::CookSTRG(CStringTable* pStringTable, IOutputStream& STRG)
{
    CStringCooker Cooker(pStringTable);

    switch (pStringTable->Game())
    {
    case EGame::PrimeDemo:
        Cooker.WritePrimeDemoSTRG(STRG);
        return true;

    case EGame::Prime:
    case EGame::EchoesDemo:
    case EGame::Echoes:
    case EGame::CorruptionProto:
        Cooker.WritePrimeSTRG(STRG);
        return true;

    case EGame::Corruption:
    case EGame::DKCReturns:
        Cooker.WriteCorruptionSTRG(STRG);
        return true;

    default:
        return false;
    }
}
