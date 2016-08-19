#ifndef CSTRINGTABLE_H
#define CSTRINGTABLE_H

#include "CResource.h"
#include <Common/CFourCC.h>
#include <Common/TString.h>
#include <Common/types.h>
#include <vector>

class CStringTable : public CResource
{
    DECLARE_RESOURCE_TYPE(eStringTable)
    friend class CStringLoader;

    std::vector<TString> mStringNames;
    u32 mNumStrings;

    struct SLangTable
    {
        CFourCC Language;
        std::vector<TWideString> Strings;
    };
    std::vector<SLangTable> mLangTables;

public:
    CStringTable(CResourceEntry *pEntry = 0) : CResource(pEntry) {}

    inline u32 NumStrings() const               { return mNumStrings; }
    inline u32 NumLanguages() const             { return mLangTables.size(); }
    inline CFourCC LanguageTag(u32 Index) const { return mLangTables[Index].Language; }
    inline TWideString String(u32 LangIndex, u32 StringIndex) const    { return mLangTables[LangIndex].Strings[StringIndex]; }
    inline TString StringName(u32 StringIndex) const                   { return mStringNames[StringIndex]; }

    TWideString String(CFourCC Lang, u32 StringIndex) const
    {
        for (u32 iLang = 0; iLang < NumLanguages(); iLang++)
        {
            if (LanguageTag(iLang) == Lang)
                return String(iLang, StringIndex);
        }

        return TWideString();
    }

    CDependencyTree* BuildDependencyTree() const
    {
        // STRGs can reference FONTs with the &font=; formatting tag and TXTRs with the &image=; tag
        CDependencyTree *pTree = new CDependencyTree(ID());
        EIDLength IDLength = (Game() <= eEchoes ? e32Bit : e64Bit);

        for (u32 iLang = 0; iLang < mLangTables.size(); iLang++)
        {
            const SLangTable& rkTable = mLangTables[iLang];

            for (u32 iStr = 0; iStr < rkTable.Strings.size(); iStr++)
            {
                const TWideString& rkStr = rkTable.Strings[iStr];

                for (u32 TagIdx = rkStr.IndexOf(L'&'); TagIdx != -1; TagIdx = rkStr.IndexOf(L'&', TagIdx + 1))
                {
                    // Check for double ampersand (escape character in DKCR, not sure about other games)
                    if (rkStr.At(TagIdx + 1) == L'&')
                    {
                        TagIdx++;
                        continue;
                    }

                    // Get tag name and parameters
                    u32 NameEnd = rkStr.IndexOf(L'=', TagIdx);
                    u32 TagEnd = rkStr.IndexOf(L';', TagIdx);
                    if (NameEnd == -1 || TagEnd == -1) continue;

                    TWideString TagName = rkStr.SubString(TagIdx + 1, NameEnd - TagIdx - 1);
                    TWideString ParamString = rkStr.SubString(NameEnd + 1, TagEnd - NameEnd - 1);

                    // Font
                    if (TagName == L"font")
                    {
                        ASSERT(ParamString.Size() == IDLength * 2);
                        pTree->AddDependency( CAssetID::FromString(ParamString) );
                    }

                    // Image
                    else if (TagName == L"image")
                    {
                        // Determine which params are textures based on image type
                        TWideStringList Params = ParamString.Split(L",");
                        TWideString ImageType = Params.front();
                        u32 TexturesStart = -1;

                        if (ImageType == L"A")
                            TexturesStart = 2;

                        else if (ImageType == L"SI")
                            TexturesStart = 3;

                        else if (ImageType == L"SA")
                            TexturesStart = 4;

                        else if (ImageType.IsHexString(false, IDLength * 2))
                            TexturesStart = 0;

                        else
                        {
                            Log::Error("Unrecognized image type: " + ImageType.ToUTF8());
                            DEBUG_BREAK;
                            continue;
                        }

                        // Load texture IDs
                        TWideStringList::iterator Iter = Params.begin();

                        for (u32 iParam = 0; iParam < Params.size(); iParam++, Iter++)
                        {
                            if (iParam >= TexturesStart)
                            {
                                TWideString Param = *Iter;
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
};

#endif // CSTRINGTABLE_H
