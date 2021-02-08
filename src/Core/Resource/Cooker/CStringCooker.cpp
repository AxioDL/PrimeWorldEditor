#include "CStringCooker.h"
#include <Common/NBasics.h>
#include <algorithm>

void CStringCooker::WritePrimeDemoSTRG(IOutputStream& STRG)
{
    const uint32 StartOffset = STRG.Tell();
    const size_t NumStrings = mpStringTable->NumStrings();

    // Start writing the file...
    STRG.WriteLong(0); // Dummy file size
    const uint32 TableStart = STRG.Tell();
    STRG.WriteLong(NumStrings);

    // Dummy string offsets
    for (size_t StringIdx = 0; StringIdx < NumStrings; StringIdx++)
        STRG.WriteLong(0);

    // Write strings
    std::vector<uint32> StringOffsets(NumStrings);

    for (size_t StringIdx = 0; StringIdx < NumStrings; StringIdx++)
    {
        StringOffsets[StringIdx] = STRG.Tell() - TableStart;
        STRG.Write16String(mpStringTable->GetString(ELanguage::English, StringIdx).ToUTF16());
    }

    // Fill in offsets
    const uint32 FileSize = STRG.Tell() - StartOffset;
    STRG.GoTo(StartOffset);
    STRG.WriteULong(FileSize);
    STRG.Skip(4);

    for (size_t StringIdx = 0; StringIdx < NumStrings; StringIdx++)
        STRG.WriteULong(StringOffsets[StringIdx]);
}

void CStringCooker::WritePrimeSTRG(IOutputStream& STRG)
{
    // Magic/Version
    STRG.WriteULong(0x87654321);
    STRG.WriteULong(mpStringTable->Game() >= EGame::EchoesDemo ? 1 : 0);
    STRG.WriteULong(static_cast<uint32>(mpStringTable->NumLanguages()));
    STRG.WriteULong(static_cast<uint32>(mpStringTable->NumStrings()));

    // Language info
    const uint32 LanguagesStart = STRG.Tell();

    for (size_t i = 0; i < mpStringTable->NumLanguages(); i++)
    {
        const CStringTable::SLanguageData& kLanguage = mpStringTable->mLanguages[i];
        STRG.WriteLong(static_cast<int>(kLanguage.Language));
        STRG.WriteLong(0); // Dummy offset

        if (mpStringTable->Game() >= EGame::EchoesDemo)
        {
            STRG.WriteLong(0); // Dummy size
        }
    }

    // Name Table
    if (mpStringTable->Game() >= EGame::EchoesDemo)
    {
        WriteNameTable(STRG);
    }

    // Strings
    const uint32 StringDataStart = STRG.Tell();
    std::vector<uint32> LanguageOffsets(mpStringTable->NumLanguages());
    std::vector<uint32> LanguageSizes(mpStringTable->NumLanguages());

    for (size_t LanguageIdx = 0; LanguageIdx < mpStringTable->NumLanguages(); LanguageIdx++)
    {
        const CStringTable::SLanguageData& kLanguage = mpStringTable->mLanguages[LanguageIdx];

        const uint32 LanguageStart = STRG.Tell();
        LanguageOffsets[LanguageIdx] = LanguageStart - StringDataStart;

        if (mpStringTable->Game() == EGame::Prime)
        {
            STRG.WriteLong(0); // Dummy size
        }

        // Fill dummy string offsets
        const uint32 StringOffsetBase = STRG.Tell();

        for (size_t StringIdx = 0; StringIdx < mpStringTable->NumStrings(); StringIdx++)
        {
            STRG.WriteLong(0);
        }

        // Write strings
        std::vector<uint32> StringOffsets(mpStringTable->NumStrings());

        for (size_t i = 0; i < mpStringTable->NumStrings(); i++)
        {
            StringOffsets[i] = STRG.Tell() - StringOffsetBase;
            STRG.Write16String(kLanguage.Strings[i].String.ToUTF16());
        }

        // Go back and fill in size/offsets
        const uint32 LanguageEnd = STRG.Tell();
        LanguageSizes[LanguageIdx] = LanguageEnd - StringOffsetBase;
        STRG.GoTo(LanguageStart);

        if (mpStringTable->Game() == EGame::Prime)
        {
            STRG.WriteULong(LanguageSizes[LanguageIdx]);
        }

        for (size_t i = 0; i < mpStringTable->NumStrings(); i++)
        {
            STRG.WriteULong(StringOffsets[i]);
        }

        STRG.GoTo(LanguageEnd);
    }

    const uint32 STRGEnd = STRG.Tell();

    // Fill in missing language data
    STRG.GoTo(LanguagesStart);

    for (size_t i = 0; i < mpStringTable->NumLanguages(); i++)
    {
        STRG.Skip(4); // Skip language ID
        STRG.WriteULong(LanguageOffsets[i]);

        if (mpStringTable->Game() >= EGame::EchoesDemo)
        {
            STRG.WriteULong(LanguageSizes[i]);
        }
    }

    STRG.GoTo(STRGEnd);
}

void CStringCooker::WriteCorruptionSTRG(IOutputStream& STRG)
{
    // Magic/Version
    STRG.WriteULong(0x87654321);
    STRG.WriteULong(3);
    STRG.WriteULong(static_cast<uint32>(mpStringTable->NumLanguages()));
    STRG.WriteULong(static_cast<uint32>(mpStringTable->NumStrings()));

    // Name Table
    WriteNameTable(STRG);

    // Create some structures before continuing...
    // In MP3 and DKCR, if a string has not been localized, then the English text
    // is reused, instead of duplicating the string data like MP1 and MP2 would have.
    struct SCookedLanguageData
    {
        ELanguage Language;
        std::vector<uint> StringOffsets;
        uint32 TotalSize;
    };
    std::vector<SCookedLanguageData> CookedLanguageData( mpStringTable->NumLanguages() );
    size_t EnglishIdx = UINT32_MAX;

    for (size_t LanguageIdx = 0; LanguageIdx < mpStringTable->NumLanguages(); LanguageIdx++)
    {
        const CStringTable::SLanguageData& kLanguageData = mpStringTable->mLanguages[LanguageIdx];

        SCookedLanguageData& CookedData = CookedLanguageData[LanguageIdx];
        CookedData.Language = kLanguageData.Language;
        CookedData.StringOffsets.resize(mpStringTable->NumStrings());
        CookedData.TotalSize = 0;

        if (CookedData.Language == ELanguage::English)
        {
            EnglishIdx = LanguageIdx;
        }
    }

    // Language IDs
    for (size_t LanguageIdx = 0; LanguageIdx < mpStringTable->NumLanguages(); LanguageIdx++)
    {
        STRG.WriteLong(static_cast<int>(CookedLanguageData[LanguageIdx].Language));
    }

    // Language Info
    const uint32 LanguageInfoStart = STRG.Tell();

    for (size_t LanguageIdx = 0; LanguageIdx < mpStringTable->NumLanguages(); LanguageIdx++)
    {
        // Fill language size/offsets with dummy data...
        STRG.WriteLong(0);

        for (size_t StringIdx = 0; StringIdx < mpStringTable->NumStrings(); StringIdx++)
            STRG.WriteLong(0);
    }

    // Strings
    const uint32 StringsStart = STRG.Tell();

    for (size_t StringIdx = 0; StringIdx < mpStringTable->NumStrings(); StringIdx++)
    {
        for (size_t LanguageIdx = 0; LanguageIdx < mpStringTable->NumLanguages(); LanguageIdx++)
        {
            const CStringTable::SLanguageData& kLanguageData = mpStringTable->mLanguages[LanguageIdx];
            const CStringTable::SStringData& kStringData = kLanguageData.Strings[StringIdx];
            SCookedLanguageData& CookedData = CookedLanguageData[LanguageIdx];

            // If the "localized" flag is disabled, then we will not write this string. Instead, it will
            // reuse the offset for the English text.
            if (LanguageIdx == EnglishIdx || kStringData.IsLocalized)
            {
                CookedData.StringOffsets[StringIdx] = STRG.Tell() - StringsStart;
                CookedData.TotalSize += kStringData.String.Size() + 1; // +1 for terminating zero
                STRG.WriteULong(kStringData.String.Size() + 1);
                STRG.WriteString(kStringData.String);
            }
            else
            {
                CookedData.StringOffsets[StringIdx] = CookedLanguageData[EnglishIdx].StringOffsets[StringIdx];
                CookedData.TotalSize += mpStringTable->mLanguages[EnglishIdx].Strings[StringIdx].String.Size() + 1; // +1 for terminating zero
            }
        }
    }

    const uint32 STRGEnd = STRG.Tell();

    // Fill in missing language data
    STRG.GoTo(LanguageInfoStart);

    for (size_t LanguageIdx = 0; LanguageIdx < mpStringTable->NumLanguages(); LanguageIdx++)
    {
        const SCookedLanguageData& kCookedData = CookedLanguageData[LanguageIdx];
        STRG.WriteULong(kCookedData.TotalSize);

        for (size_t StringIdx = 0; StringIdx < mpStringTable->NumStrings(); StringIdx++)
            STRG.WriteULong(kCookedData.StringOffsets[StringIdx]);
    }

    STRG.GoTo(STRGEnd);
}

void CStringCooker::WriteNameTable(IOutputStream& STRG)
{
    // Build a list of name entries to put in the map
    struct SNameEntry
    {
        uint32 Offset;
        uint32 Index;
        TString Name;
    };
    std::vector<SNameEntry> NameEntries;

    for (uint32 StringIdx = 0; StringIdx < mpStringTable->NumStrings(); StringIdx++)
    {
        SNameEntry Entry;
        Entry.Offset = 0;
        Entry.Index = StringIdx;
        Entry.Name = mpStringTable->StringNameByIndex(StringIdx);

        if (!Entry.Name.IsEmpty())
        {
            NameEntries.push_back(std::move(Entry));
        }
    }

    std::stable_sort(NameEntries.begin(), NameEntries.end(), [](const SNameEntry& kLHS, const SNameEntry& kRHS) {
        return kLHS.Name < kRHS.Name;
    });

    // Write out name entries
    const uint32 NameTableStart = STRG.Tell();
    STRG.WriteULong(static_cast<uint32>(NameEntries.size()));
    STRG.WriteULong(0); // Dummy name table size
    const uint32 NameTableOffsetsStart = STRG.Tell();

    for (const auto& entry : NameEntries)
    {
        STRG.WriteULong(0); // Dummy name offset
        STRG.WriteULong(entry.Index);
    }

    // Write out names
    std::vector<uint32> NameOffsets(NameEntries.size());

    for (size_t NameIdx = 0; NameIdx < NameEntries.size(); NameIdx++)
    {
        NameOffsets[NameIdx] = STRG.Tell() - NameTableOffsetsStart;
        STRG.WriteString(NameEntries[NameIdx].Name);
    }

    // Fill out sizes and offsets
    const uint32 NameTableEnd = STRG.Tell();
    const uint32 NameTableSize = NameTableEnd - NameTableOffsetsStart;

    STRG.GoTo(NameTableStart);
    STRG.Skip(4);
    STRG.WriteULong(NameTableSize);

    for (const uint32 offset : NameOffsets)
    {
        STRG.WriteULong(offset);
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
