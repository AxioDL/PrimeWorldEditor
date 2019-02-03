#include "CStringTable.h"
#include "Core/GameProject/CGameProject.h"
#include <Common/Math/MathUtil.h>
#include <algorithm>
#include <iterator>

/**
 * Listing of supported languages for different engine versions. Note we ignore the "unused" languages.
 * This is also the order that languages appear in game STRG assets.
 */
// Supported languages in the original NTSC release of Metroid Prime
const std::vector<ELanguage> gkSupportedLanguagesMP1 =
{
    ELanguage::English
};

// Supported languages in the PAL version of Metroid Prime, and also Metroid Prime 2
const std::vector<ELanguage> gkSupportedLanguagesMP1PAL =
{
    ELanguage::English, ELanguage::French, ELanguage::German,
    ELanguage::Spanish, ELanguage::Italian, ELanguage::Japanese
};

// Supported languages in Metroid Prime 3
const std::vector<ELanguage> gkSupportedLanguagesMP3 =
{
    ELanguage::English, ELanguage::Japanese, ELanguage::German,
    ELanguage::French, ELanguage::Spanish, ELanguage::Italian
};

// Supported languages in DKCR
const std::vector<ELanguage> gkSupportedLanguagesDKCR =
{
    ELanguage::English, ELanguage::Japanese, ELanguage::German,
    ELanguage::French, ELanguage::Spanish, ELanguage::Italian,
    ELanguage::UKEnglish, ELanguage::Korean,
    ELanguage::NAFrench, ELanguage::NASpanish
};

// Utility function - retrieve the language array for a given game/region
static const std::vector<ELanguage>& GetSupportedLanguages(EGame Game, ERegion Region)
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
    for (uint LanguageIdx = 0; LanguageIdx < pkInTable->NumLanguages(); LanguageIdx++)
    {
        if (pkInTable->LanguageByIndex(LanguageIdx) == InLanguage)
        {
            return LanguageIdx;
        }
    }

    return -1;
}

/** Returns a string given a language/index pair */
TString CStringTable::GetString(ELanguage Language, uint StringIndex) const
{
    int LanguageIdx = FindLanguageIndex(this, Language);

    if (LanguageIdx >= 0 && mLanguages[LanguageIdx].Strings.size() > StringIndex)
    {
        return mLanguages[LanguageIdx].Strings[StringIndex].String;
    }
    else
    {
        return "";
    }
}

/** Updates a string for a given language */
void CStringTable::SetString(ELanguage Language, uint StringIndex, const TString& kNewString)
{
    int LanguageIdx = FindLanguageIndex(this, Language);

    if (LanguageIdx >= 0 && mLanguages[LanguageIdx].Strings.size() > StringIndex)
    {
        mLanguages[LanguageIdx].Strings[StringIndex].String = kNewString;
        mLanguages[LanguageIdx].Strings[StringIndex].IsLocalized =
            (LanguageIdx == 0 || kNewString != mLanguages[0].Strings[StringIndex].String);
    }
}

/** Updates a string name */
void CStringTable::SetStringName(uint StringIndex, const TString& kNewName)
{
    // Sanity check - make sure the string index is valid
    ASSERT( NumStrings() > StringIndex );

    // Expand the name listing if needed and assign the name
    if (mStringNames.size() <= StringIndex)
    {
        mStringNames.resize( StringIndex + 1 );
    }

    mStringNames[StringIndex] = kNewName;

    // Strip empty string names
    while (mStringNames.back().IsEmpty())
        mStringNames.pop_back();
}

/** Move string to another position in the table */
void CStringTable::MoveString(uint StringIndex, uint NewIndex)
{
    ASSERT( NumStrings() > StringIndex );
    ASSERT( NumStrings() > NewIndex );

    if (NewIndex == StringIndex)
        return;

    // Update string data
    for (uint LanguageIdx = 0; LanguageIdx < mLanguages.size(); LanguageIdx++)
    {
        SLanguageData& Language = mLanguages[LanguageIdx];
        SStringData String = Language.Strings[StringIndex];

        if (NewIndex > StringIndex)
        {
            for (uint i=StringIndex; i<NewIndex; i++)
                Language.Strings[i] = Language.Strings[i+1];
        }
        else
        {
            for (uint i=StringIndex; i>NewIndex; i--)
                Language.Strings[i] = Language.Strings[i-1];
        }

        Language.Strings[NewIndex] = String;
    }

    // Update string name
    uint MinIndex = Math::Min(StringIndex, NewIndex);
    uint MaxIndex = Math::Max(StringIndex, NewIndex);

    if (MinIndex < mStringNames.size())
    {
        if (MaxIndex >= mStringNames.size())
        {
            mStringNames.resize(MaxIndex + 1);
        }

        TString Name = mStringNames[StringIndex];

        if (NewIndex > StringIndex)
        {
            for (uint i=StringIndex; i<NewIndex; i++)
                mStringNames[i] = mStringNames[i+1];
        }
        else
        {
            for (uint i=StringIndex; i>NewIndex; i--)
                mStringNames[i] = mStringNames[i-1];
        }
        mStringNames[NewIndex] = Name;

        // Strip empty string names
        while (mStringNames.back().IsEmpty())
            mStringNames.pop_back();
    }
}

/** Add a new string to the table */
void CStringTable::AddString(uint AtIndex)
{
    if (AtIndex < NumStrings())
    {
        if (mStringNames.size() > AtIndex)
        {
            mStringNames.insert( mStringNames.begin() + AtIndex, 1,  "" );
        }
    }
    else
        AtIndex = NumStrings();

    for (uint LanguageIdx = 0; LanguageIdx < mLanguages.size(); LanguageIdx++)
    {
        SLanguageData& Language = mLanguages[LanguageIdx];
        Language.Strings.insert( Language.Strings.begin() + AtIndex, 1, SStringData() );
    }
}

/** Remove a string from the table */
void CStringTable::RemoveString(uint StringIndex)
{
    ASSERT( StringIndex < NumStrings() );

    if (mStringNames.size() > StringIndex)
        mStringNames.erase( mStringNames.begin() + StringIndex );

    for (uint LanguageIdx = 0; LanguageIdx < mLanguages.size(); LanguageIdx++)
    {
        SLanguageData& Language = mLanguages[LanguageIdx];
        Language.Strings.erase( Language.Strings.begin() + StringIndex );
    }
}

/** Initialize new resource data */
void CStringTable::InitializeNewResource()
{
    // Initialize data for whatever languages are supported by our game/region
    ERegion Region = ( Entry() && Entry()->Project() ? Entry()->Project()->Region() : ERegion::NTSC );
    const std::vector<ELanguage>& kLanguageArray = GetSupportedLanguages(Game(), Region);
    mLanguages.resize( kLanguageArray.size() );

    for (uint i=0; i < kLanguageArray.size(); i++)
    {
        mLanguages[i].Language = kLanguageArray[i];
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
CDependencyTree* CStringTable::BuildDependencyTree() const
{
    // STRGs can reference FONTs with the &font=; formatting tag and TXTRs with the &image=; tag
    CDependencyTree* pTree = new CDependencyTree();
    EIDLength IDLength = CAssetID::GameIDLength( Game() );

    for (uint LanguageIdx = 0; LanguageIdx < mLanguages.size(); LanguageIdx++)
    {
        const SLanguageData& kLanguage = mLanguages[LanguageIdx];

        for (uint StringIdx = 0; StringIdx < kLanguage.Strings.size(); StringIdx++)
        {
            const TString& kString = kLanguage.Strings[StringIdx].String;

            for (int TagIdx = kString.IndexOf('&'); TagIdx != -1; TagIdx = kString.IndexOf('&', TagIdx + 1))
            {
                // Check for double ampersand (escape character in DKCR, not sure about other games)
                if (kString.At(TagIdx + 1) == '&')
                {
                    TagIdx++;
                    continue;
                }

                // Get tag name and parameters
                int NameEnd = kString.IndexOf('=', TagIdx);
                int TagEnd = kString.IndexOf(';', TagIdx);
                if (NameEnd == -1 || TagEnd == -1) continue;

                TString TagName = kString.SubString(TagIdx + 1, NameEnd - TagIdx - 1);
                TString ParamString = kString.SubString(NameEnd + 1, TagEnd - NameEnd - 1);
                if (ParamString.IsEmpty()) continue;

                // Font
                if (TagName == "font")
                {
                    if (Game() >= EGame::CorruptionProto)
                    {
                        ASSERT(ParamString.StartsWith("0x"));
                        ParamString = ParamString.ChopFront(2);
                    }

                    ASSERT(ParamString.Size() == IDLength * 2);
                    pTree->AddDependency( CAssetID::FromString(ParamString) );
                }

                // Image
                else if (TagName == "image")
                {
                    // Determine which params are textures based on image type
                    TStringList Params = ParamString.Split(",");
                    TString ImageType = Params.front();
                    uint TexturesStart = 0;

                    if (ImageType == "A")
                        TexturesStart = 2;

                    else if (ImageType == "SI")
                        TexturesStart = 3;

                    else if (ImageType == "SA")
                        TexturesStart = 4;

                    else if (ImageType == "B")
                        TexturesStart = 2;

                    else if (ImageType.IsHexString(false, IDLength * 2))
                        TexturesStart = 0;

                    else
                    {
                        errorf("Unrecognized image type: %s", *ImageType);
                        continue;
                    }

                    // Load texture IDs
                    TStringList::iterator Iter = Params.begin();

                    for (uint ParamIdx = 0; ParamIdx < Params.size(); ParamIdx++, Iter++)
                    {
                        if (ParamIdx >= TexturesStart)
                        {
                            TString Param = *Iter;

                            if (Game() >= EGame::CorruptionProto)
                            {
                                ASSERT(Param.StartsWith("0x"));
                                Param = Param.ChopFront(2);
                            }

                            ASSERT(Param.Size() == IDLength * 2);
                            pTree->AddDependency( CAssetID::FromString(Param) );
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
    int TagStart = -1;

    for (uint CharIdx = 0; CharIdx < Out.Size(); CharIdx++)
    {
        if (Out[CharIdx] == '&')
        {
            if (TagStart == -1)
                TagStart = CharIdx;

            else
            {
                Out.Remove(TagStart, 1);
                TagStart = -1;
                CharIdx--;
            }
        }

        else if (TagStart != -1 && Out[CharIdx] == ';')
        {
            int TagEnd = CharIdx + 1;
            int TagLen = TagEnd - TagStart;
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
    const std::vector<ELanguage>& kLanguageArray = GetSupportedLanguages(Game, Region);

    // Check if the requested language is in the array.
    for (uint LanguageIdx = 0; LanguageIdx < kLanguageArray.size(); LanguageIdx++)
    {
        if (kLanguageArray[LanguageIdx] == Language)
        {
            return true;
        }
    }

    // Unsupported
    return false;
}
