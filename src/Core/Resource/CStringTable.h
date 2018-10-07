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
        std::vector<TString> Strings;
    };
    std::vector<SLangTable> mLangTables;

public:
    CStringTable(CResourceEntry *pEntry = 0) : CResource(pEntry) {}

    inline u32 NumStrings() const               { return mNumStrings; }
    inline u32 NumLanguages() const             { return mLangTables.size(); }
    inline CFourCC LanguageTag(u32 Index) const { return mLangTables[Index].Language; }
    inline TString String(u32 LangIndex, u32 StringIndex) const { return mLangTables[LangIndex].Strings[StringIndex]; }
    inline TString StringName(u32 StringIndex) const            { return mStringNames[StringIndex]; }

    TString String(CFourCC Lang, u32 StringIndex) const
    {
        for (u32 iLang = 0; iLang < NumLanguages(); iLang++)
        {
            if (LanguageTag(iLang) == Lang)
                return String(iLang, StringIndex);
        }

        return TString();
    }

    CDependencyTree* BuildDependencyTree() const
    {
        // STRGs can reference FONTs with the &font=; formatting tag and TXTRs with the &image=; tag
        CDependencyTree *pTree = new CDependencyTree();
        EIDLength IDLength = (Game() <= EGame::Echoes ? e32Bit : e64Bit);

        for (u32 iLang = 0; iLang < mLangTables.size(); iLang++)
        {
            const SLangTable& rkTable = mLangTables[iLang];

            for (u32 iStr = 0; iStr < rkTable.Strings.size(); iStr++)
            {
                const TString& rkStr = rkTable.Strings[iStr];

                for (u32 TagIdx = rkStr.IndexOf('&'); TagIdx != -1; TagIdx = rkStr.IndexOf('&', TagIdx + 1))
                {
                    // Check for double ampersand (escape character in DKCR, not sure about other games)
                    if (rkStr.At(TagIdx + 1) == '&')
                    {
                        TagIdx++;
                        continue;
                    }

                    // Get tag name and parameters
                    u32 NameEnd = rkStr.IndexOf('=', TagIdx);
                    u32 TagEnd = rkStr.IndexOf(';', TagIdx);
                    if (NameEnd == -1 || TagEnd == -1) continue;

                    TString TagName = rkStr.SubString(TagIdx + 1, NameEnd - TagIdx - 1);
                    TString ParamString = rkStr.SubString(NameEnd + 1, TagEnd - NameEnd - 1);
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
                        u32 TexturesStart = -1;

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
                            Log::Error("Unrecognized image type: " + ImageType);
                            DEBUG_BREAK;
                            continue;
                        }

                        // Load texture IDs
                        TStringList::iterator Iter = Params.begin();

                        for (u32 iParam = 0; iParam < Params.size(); iParam++, Iter++)
                        {
                            if (iParam >= TexturesStart)
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

    static TString StripFormatting(const TString& rkStr)
    {
        TString Out = rkStr;
        int TagStart = -1;

        for (u32 iChr = 0; iChr < Out.Size(); iChr++)
        {
            if (Out[iChr] == '&')
            {
                if (TagStart == -1)
                    TagStart = iChr;

                else
                {
                    Out.Remove(TagStart, 1);
                    TagStart = -1;
                    iChr--;
                }
            }

            else if (TagStart != -1 && Out[iChr] == ';')
            {
                int TagEnd = iChr + 1;
                int TagLen = TagEnd - TagStart;
                Out.Remove(TagStart, TagLen);
                iChr = TagStart - 1;
                TagStart = -1;
            }
        }

        return Out;
    }
};

#endif // CSTRINGTABLE_H
