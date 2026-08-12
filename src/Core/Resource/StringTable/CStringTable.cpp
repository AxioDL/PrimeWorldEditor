#include "Core/Resource/StringTable/CStringTable.h"

#include "Core/GameProject/CGameProject.h"
#include <Common/Log.h>
#include <Common/Macros.h>
#include <algorithm>
#include <array>
#include <span>
#include <utility>

/**
 * Listing of supported languages for different engine versions. Note we ignore the "unused" languages.
 * This is also the order that languages appear in game STRG assets.
 */
// Supported languages in the original NTSC release of Metroid Prime
constexpr std::array gkSupportedLanguagesMP1{
    ELanguage::English,
};

// Supported languages in the PAL version of Metroid Prime, and also Metroid Prime 2
constexpr std::array gkSupportedLanguagesMP1PAL{
    ELanguage::English,
    ELanguage::French,
    ELanguage::German,
    ELanguage::Spanish,
    ELanguage::Italian,
    ELanguage::Japanese,
};

// Supported languages in Metroid Prime 3
constexpr std::array gkSupportedLanguagesMP3{
    ELanguage::English,
    ELanguage::Japanese,
    ELanguage::German,
    ELanguage::French,
    ELanguage::Spanish,
    ELanguage::Italian,
};

// Supported languages in DKCR
constexpr std::array gkSupportedLanguagesDKCR{
    ELanguage::English,
    ELanguage::Japanese,
    ELanguage::German,
    ELanguage::French,
    ELanguage::Spanish,
    ELanguage::Italian,
    ELanguage::UKEnglish,
    ELanguage::Korean,
    ELanguage::NAFrench,
    ELanguage::NASpanish,
};

// Utility function - retrieve the language array for a given game/region
static std::span<const ELanguage> GetSupportedLanguages(EGame Game, ERegion Region)
{
    switch (Game)
    {
    default:
    case EGame::PrimeDemo:
    case EGame::Prime:
        if (Region == ERegion::NTSC)
            return gkSupportedLanguagesMP1;
        else
            return gkSupportedLanguagesMP1PAL;

    case EGame::EchoesDemo:
    case EGame::Echoes:
    case EGame::CorruptionProto:
        return gkSupportedLanguagesMP1PAL;

    case EGame::Corruption:
        return gkSupportedLanguagesMP3;

    case EGame::DKCReturns:
        return gkSupportedLanguagesDKCR;
    }
}

// Utility function - retrieve the index of a given language
static int FindLanguageIndex(const CStringTable* pkInTable, ELanguage InLanguage)
{
    for (size_t LanguageIdx = 0; LanguageIdx < pkInTable->NumLanguages(); LanguageIdx++)
    {
        if (pkInTable->LanguageByIndex(LanguageIdx) == InLanguage)
        {
            return static_cast<int>(LanguageIdx);
        }
    }

    return -1;
}

CStringTable::CStringTable(CResourceEntry* pEntry)
    : CResource(pEntry) {}

CStringTable::~CStringTable() = default;

/** Returns a string given a language/index pair */
TString CStringTable::GetString(ELanguage Language, size_t StringIndex) const
{
    const int LanguageIdx = FindLanguageIndex(this, Language);

    if (LanguageIdx >= 0 && mLanguages[LanguageIdx].Strings.size() > StringIndex)
    {
        return mLanguages[LanguageIdx].Strings[StringIndex].String;
    }

    return "";
}

/** Updates a string for a given language */
void CStringTable::SetString(ELanguage Language, size_t StringIndex, TString kNewString)
{
    const int LanguageIdx = FindLanguageIndex(this, Language);
    const int EnglishIdx = FindLanguageIndex(this, ELanguage::English);

    if (LanguageIdx >= 0 && mLanguages[LanguageIdx].Strings.size() > StringIndex)
    {
        auto& string = mLanguages[LanguageIdx].Strings[StringIndex];

        string.IsLocalized = LanguageIdx == EnglishIdx || kNewString != mLanguages[EnglishIdx].Strings[StringIndex].String;
        string.String = std::move(kNewString);
    }
}

/** Updates a string name */
void CStringTable::SetStringName(size_t StringIndex, TString kNewName)
{
    // Sanity check - make sure the string index is valid
    ASSERT(NumStrings() > StringIndex);

    // Expand the name listing if needed and assign the name
    if (mStringNames.size() <= StringIndex)
    {
        mStringNames.resize(StringIndex + 1);
    }

    mStringNames[StringIndex] = std::move(kNewName);

    // Strip empty string names
    while (!mStringNames.empty() && mStringNames.back().IsEmpty())
        mStringNames.pop_back();
}

/** Move string to another position in the table */
void CStringTable::MoveString(size_t StringIndex, size_t NewIndex)
{
    ASSERT(NumStrings() > StringIndex);
    ASSERT(NumStrings() > NewIndex);

    if (NewIndex == StringIndex)
        return;

    // Update string data
    for (SLanguageData& Language : mLanguages)
    {
        SStringData String = Language.Strings[StringIndex];

        if (NewIndex > StringIndex)
        {
            for (size_t i = StringIndex; i < NewIndex; i++)
                Language.Strings[i] = Language.Strings[i + 1];
        }
        else
        {
            for (size_t i = StringIndex; i > NewIndex; i--)
                Language.Strings[i] = Language.Strings[i - 1];
        }

        Language.Strings[NewIndex] = std::move(String);
    }

    // Update string name
    const auto [MinIndex, MaxIndex] = std::minmax(StringIndex, NewIndex);

    if (MinIndex < mStringNames.size())
    {
        if (MaxIndex >= mStringNames.size())
        {
            mStringNames.resize(MaxIndex + 1);
        }

        TString Name = mStringNames[StringIndex];

        if (NewIndex > StringIndex)
        {
            for (size_t i = StringIndex; i < NewIndex; i++)
                mStringNames[i] = mStringNames[i + 1];
        }
        else
        {
            for (size_t i = StringIndex; i > NewIndex; i--)
                mStringNames[i] = mStringNames[i - 1];
        }
        mStringNames[NewIndex] = std::move(Name);

        // Strip empty string names
        while (mStringNames.back().IsEmpty())
            mStringNames.pop_back();
    }
}

/** Add a new string to the table */
void CStringTable::AddString(size_t AtIndex)
{
    if (AtIndex < NumStrings())
    {
        if (mStringNames.size() > AtIndex)
        {
            mStringNames.emplace(mStringNames.begin() + AtIndex);
        }
    }
    else
    {
        AtIndex = NumStrings();
    }

    for (SLanguageData& Language : mLanguages)
    {
        Language.Strings.emplace(Language.Strings.begin() + AtIndex);
    }
}

/** Remove a string from the table */
void CStringTable::RemoveString(size_t StringIndex)
{
    ASSERT(StringIndex < NumStrings());

    if (mStringNames.size() > StringIndex)
        mStringNames.erase(mStringNames.begin() + StringIndex);

    for (SLanguageData& Language : mLanguages)
    {
        Language.Strings.erase(Language.Strings.begin() + StringIndex);
    }
}

/** Initialize new resource data */
void CStringTable::InitializeNewResource()
{
    // Initialize data for whatever languages are supported by our game/region
    const ERegion Region = (Entry() && Entry()->Project() ? Entry()->Project()->Region() : ERegion::NTSC);
    const auto langs = GetSupportedLanguages(Game(), Region);
    mLanguages.resize(langs.size());

    for (size_t i = 0; i < langs.size(); i++)
    {
        mLanguages[i].Language = langs[i];
        mLanguages[i].Strings.resize(1);
    }
}

/** Serialize resource data */
void CStringTable::Serialize(IArchive& Arc)
{
    Arc << SerialParameter("StringNames", mStringNames, SH_Optional)
        << SerialParameter("Languages", mLanguages);
}

/** Build the dependency tree for this resource */
std::unique_ptr<CDependencyTree> CStringTable::BuildDependencyTree()
{
    // STRGs can reference FONTs with the &font=; formatting tag and TXTRs with the &image=; tag
    auto pTree = std::make_unique<CDependencyTree>();
    EIDLength IDLength = CAssetID::GameIDLength(Game());

    for (const SLanguageData& language : mLanguages)
    {
        for (const auto& stringData : language.Strings)
        {
            const TString& kString = stringData.String;

            for (auto TagIdx = kString.IndexOf('&'); TagIdx != -1; TagIdx = kString.IndexOf('&', TagIdx + 1))
            {
                // Check for double ampersand (escape character in DKCR, not sure about other games)
                if (kString.At(TagIdx + 1) == '&')
                {
                    TagIdx++;
                    continue;
                }

                // Get tag name and parameters
                const auto NameEnd = kString.IndexOf('=', TagIdx);
                const auto TagEnd = kString.IndexOf(';', TagIdx);
                if (NameEnd == -1 || TagEnd == -1)
                    continue;

                const TString TagName = kString.SubString(TagIdx + 1, NameEnd - TagIdx - 1);
                TString ParamString = kString.SubString(NameEnd + 1, TagEnd - NameEnd - 1);
                if (ParamString.IsEmpty())
                    continue;

                // Font
                if (TagName == "font")
                {
                    if (Game() >= EGame::CorruptionProto)
                    {
                        if (!ParamString.StartsWith("0x"))
                            continue;
                        ParamString = ParamString.ChopFront(2);
                    }

                    ASSERT(ParamString.Size() == size_t(IDLength) * 2);
                    pTree->AddDependency(CAssetID::FromString(ParamString));
                }
                // Image
                else if (TagName == "image")
                {
                    // Determine which params are textures based on image type
                    TStringList Params = ParamString.Split(",");
                    const TString& ImageType = Params.front();
                    size_t TexturesStart = 0;

                    if (ImageType == "A" || ImageType == "B")
                    {
                        TexturesStart = 2;
                    }
                    else if (ImageType == "SI")
                    {
                        TexturesStart = 3;
                    }
                    else if (ImageType == "SA")
                    {
                        TexturesStart = 4;
                    }
                    else if (ImageType.IsHexString(false, static_cast<int>(IDLength) * 2))
                    {
                        TexturesStart = 0;
                    }
                    else
                    {
                        NLog::Warn("Unrecognized image type: {}", ImageType);
                        continue;
                    }

                    // Load texture IDs
                    auto Iter = Params.begin();

                    for (size_t ParamIdx = 0; ParamIdx < Params.size(); ParamIdx++, ++Iter)
                    {
                        if (ParamIdx >= TexturesStart)
                        {
                            TString Param = *Iter;

                            if (Game() >= EGame::CorruptionProto)
                            {
                                ASSERT(Param.StartsWith("0x"));
                                Param = Param.ChopFront(2);
                            }

                            ASSERT(Param.Size() == size_t(IDLength) * 2);
                            pTree->AddDependency(CAssetID::FromString(Param));
                        }
                    }
                }
            }
        }
    }

    return pTree;
}

/** Static - Strip all formatting tags for a given string */
TString CStringTable::StripFormatting(const TString& kInString)
{
    TString Out = kInString;
    int64_t TagStart = -1;

    for (size_t CharIdx = 0; CharIdx < Out.Size(); CharIdx++)
    {
        if (Out[CharIdx] == '&')
        {
            if (TagStart == -1)
            {
                TagStart = CharIdx;
            }
            else
            {
                Out.Remove(TagStart, 1);
                TagStart = -1;
                CharIdx--;
            }
        }
        else if (TagStart != -1 && Out[CharIdx] == ';')
        {
            const int64_t TagEnd = CharIdx + 1;
            const int64_t TagLen = TagEnd - TagStart;
            Out.Remove(TagStart, TagLen);
            CharIdx = TagStart - 1;
            TagStart = -1;
        }
    }

    return Out;
}

/** Static - Returns whether a given language is supported by the given game/region combination */
bool CStringTable::IsLanguageSupported(ELanguage Language, EGame Game, ERegion Region)
{
    const auto langs = GetSupportedLanguages(Game, Region);
    return std::ranges::contains(langs, Language);
}
