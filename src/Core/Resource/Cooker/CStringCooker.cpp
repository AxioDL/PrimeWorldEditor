#include "Core/Resource/Cooker/CStringCooker.h"

#include <Common/NBasics.h>
#include "Core/Resource/StringTable/CStringTable.h"

#include <algorithm>

CStringCooker::CStringCooker(CStringTable* pStringTable)
    : mpStringTable(pStringTable) {}

void CStringCooker::WritePrimeDemoSTRG(IOutputStream& STRG)
{
    const auto StartOffset = STRG.Tell();
    const auto NumStrings = mpStringTable->NumStrings();

    // Start writing the file...
    STRG.WriteU32(0); // Dummy file size
    const auto TableStart = STRG.Tell();
    STRG.WriteU32(NumStrings);

    // Dummy string offsets
    for (size_t StringIdx = 0; StringIdx < NumStrings; StringIdx++)
        STRG.WriteU32(0);

    // Write strings
    std::vector<uint32_t> StringOffsets(NumStrings);

    for (size_t StringIdx = 0; StringIdx < NumStrings; StringIdx++)
    {
        StringOffsets[StringIdx] = STRG.Tell() - TableStart;
        STRG.Write16String(mpStringTable->GetString(ELanguage::English, StringIdx).ToUTF16());
    }

    // Fill in offsets
    const auto FileSize = STRG.Tell() - StartOffset;
    STRG.GoTo(StartOffset);
    STRG.WriteU32(FileSize);
    STRG.Skip(4);

    for (size_t StringIdx = 0; StringIdx < NumStrings; StringIdx++)
        STRG.WriteU32(StringOffsets[StringIdx]);
}

void CStringCooker::WritePrimeSTRG(IOutputStream& STRG)
{
    // Magic/Version
    STRG.WriteU32(0x87654321);
    STRG.WriteU32(mpStringTable->Game() >= EGame::EchoesDemo ? 1 : 0);
    STRG.WriteU32(static_cast<uint32_t>(mpStringTable->NumLanguages()));
    STRG.WriteU32(static_cast<uint32_t>(mpStringTable->NumStrings()));

    // Language info
    const auto LanguagesStart = STRG.Tell();

    for (size_t i = 0; i < mpStringTable->NumLanguages(); i++)
    {
        const CStringTable::SLanguageData& kLanguage = mpStringTable->mLanguages[i];
        STRG.WriteU32(static_cast<uint32_t>(kLanguage.Language));
        STRG.WriteU32(0); // Dummy offset

        if (mpStringTable->Game() >= EGame::EchoesDemo)
        {
            STRG.WriteU32(0); // Dummy size
        }
    }

    // Name Table
    if (mpStringTable->Game() >= EGame::EchoesDemo)
    {
        WriteNameTable(STRG);
    }

    // Strings
    const auto StringDataStart = STRG.Tell();
    std::vector<uint32_t> LanguageOffsets(mpStringTable->NumLanguages());
    std::vector<uint32_t> LanguageSizes(mpStringTable->NumLanguages());

    for (size_t LanguageIdx = 0; LanguageIdx < mpStringTable->NumLanguages(); LanguageIdx++)
    {
        const CStringTable::SLanguageData& kLanguage = mpStringTable->mLanguages[LanguageIdx];

        const auto LanguageStart = STRG.Tell();
        LanguageOffsets[LanguageIdx] = LanguageStart - StringDataStart;

        if (mpStringTable->Game() == EGame::Prime)
        {
            STRG.WriteU32(0); // Dummy size
        }

        // Fill dummy string offsets
        const auto StringOffsetBase = STRG.Tell();

        for (size_t StringIdx = 0; StringIdx < mpStringTable->NumStrings(); StringIdx++)
        {
            STRG.WriteU32(0);
        }

        // Write strings
        std::vector<uint32_t> StringOffsets(mpStringTable->NumStrings());

        for (size_t i = 0; i < mpStringTable->NumStrings(); i++)
        {
            StringOffsets[i] = STRG.Tell() - StringOffsetBase;
            STRG.Write16String(kLanguage.Strings[i].String.ToUTF16());
        }

        // Go back and fill in size/offsets
        const auto LanguageEnd = STRG.Tell();
        LanguageSizes[LanguageIdx] = LanguageEnd - StringOffsetBase;
        STRG.GoTo(LanguageStart);

        if (mpStringTable->Game() == EGame::Prime)
        {
            STRG.WriteU32(LanguageSizes[LanguageIdx]);
        }

        for (size_t i = 0; i < mpStringTable->NumStrings(); i++)
        {
            STRG.WriteU32(StringOffsets[i]);
        }

        STRG.GoTo(LanguageEnd);
    }

    const auto STRGEnd = STRG.Tell();

    // Fill in missing language data
    STRG.GoTo(LanguagesStart);

    for (size_t i = 0; i < mpStringTable->NumLanguages(); i++)
    {
        STRG.Skip(4); // Skip language ID
        STRG.WriteU32(LanguageOffsets[i]);

        if (mpStringTable->Game() >= EGame::EchoesDemo)
        {
            STRG.WriteU32(LanguageSizes[i]);
        }
    }

    STRG.GoTo(STRGEnd);
}

void CStringCooker::WriteCorruptionSTRG(IOutputStream& STRG)
{
    // Magic/Version
    STRG.WriteU32(0x87654321);
    STRG.WriteU32(3);
    STRG.WriteU32(static_cast<uint32_t>(mpStringTable->NumLanguages()));
    STRG.WriteU32(static_cast<uint32_t>(mpStringTable->NumStrings()));

    // Name Table
    WriteNameTable(STRG);

    // Create some structures before continuing...
    // In MP3 and DKCR, if a string has not been localized, then the English text
    // is reused, instead of duplicating the string data like MP1 and MP2 would have.
    struct SCookedLanguageData
    {
        ELanguage Language{};
        std::vector<uint32_t> StringOffsets;
        uint32_t TotalSize{};
    };
    std::vector<SCookedLanguageData> CookedLanguageData(mpStringTable->NumLanguages());
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
        STRG.WriteU32(static_cast<uint32_t>(CookedLanguageData[LanguageIdx].Language));
    }

    // Language Info
    const uint32_t LanguageInfoStart = STRG.Tell();

    for (size_t LanguageIdx = 0; LanguageIdx < mpStringTable->NumLanguages(); LanguageIdx++)
    {
        // Fill language size/offsets with dummy data...
        STRG.WriteU32(0);

        for (size_t StringIdx = 0; StringIdx < mpStringTable->NumStrings(); StringIdx++)
            STRG.WriteU32(0);
    }

    // Strings
    const uint32_t StringsStart = STRG.Tell();

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
                STRG.WriteU32(kStringData.String.Size() + 1);
                STRG.WriteString(kStringData.String);
            }
            else
            {
                CookedData.StringOffsets[StringIdx] = CookedLanguageData[EnglishIdx].StringOffsets[StringIdx];
                CookedData.TotalSize += mpStringTable->mLanguages[EnglishIdx].Strings[StringIdx].String.Size() + 1; // +1 for terminating zero
            }
        }
    }

    const uint32_t STRGEnd = STRG.Tell();

    // Fill in missing language data
    STRG.GoTo(LanguageInfoStart);

    for (size_t LanguageIdx = 0; LanguageIdx < mpStringTable->NumLanguages(); LanguageIdx++)
    {
        const SCookedLanguageData& kCookedData = CookedLanguageData[LanguageIdx];
        STRG.WriteU32(kCookedData.TotalSize);

        for (size_t StringIdx = 0; StringIdx < mpStringTable->NumStrings(); StringIdx++)
            STRG.WriteU32(kCookedData.StringOffsets[StringIdx]);
    }

    STRG.GoTo(STRGEnd);
}

void CStringCooker::WriteNameTable(IOutputStream& STRG)
{
    // Build a list of name entries to put in the map
    struct SNameEntry
    {
        uint32_t Offset{};
        uint32_t Index{};
        TString Name;
    };
    std::vector<SNameEntry> NameEntries;

    for (uint32_t StringIdx = 0; StringIdx < mpStringTable->NumStrings(); StringIdx++)
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

    std::ranges::stable_sort(NameEntries, [](const SNameEntry& kLHS, const SNameEntry& kRHS) {
        return kLHS.Name < kRHS.Name;
    });

    // Write out name entries
    const auto NameTableStart = STRG.Tell();
    STRG.WriteU32(static_cast<uint32_t>(NameEntries.size()));
    STRG.WriteU32(0); // Dummy name table size
    const auto NameTableOffsetsStart = STRG.Tell();

    for (const auto& entry : NameEntries)
    {
        STRG.WriteU32(0); // Dummy name offset
        STRG.WriteU32(entry.Index);
    }

    // Write out names
    std::vector<uint32_t> NameOffsets(NameEntries.size());

    for (size_t NameIdx = 0; NameIdx < NameEntries.size(); NameIdx++)
    {
        NameOffsets[NameIdx] = STRG.Tell() - NameTableOffsetsStart;
        STRG.WriteString(NameEntries[NameIdx].Name);
    }

    // Fill out sizes and offsets
    const auto NameTableEnd = STRG.Tell();
    const auto NameTableSize = NameTableEnd - NameTableOffsetsStart;

    STRG.GoTo(NameTableStart);
    STRG.Skip(4);
    STRG.WriteU32(NameTableSize);

    for (const auto offset : NameOffsets)
    {
        STRG.WriteU32(offset);
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
