#ifndef CSTRINGTABLE_H
#define CSTRINGTABLE_H

#include "ELanguage.h"
#include "Core/Resource/CResource.h"
#include <Common/BasicTypes.h>
#include <Common/CFourCC.h>
#include <Common/TString.h>
#include <vector>

/** A table of localized strings from STRG assets.
 *  Strings are always internally stored as UTF-8.
 */
class CStringTable : public CResource
{
    DECLARE_RESOURCE_TYPE(StringTable)
    friend class CStringLoader;
    friend class CStringCooker;

    /** List of string names. Optional data, can be empty. */
    std::vector<TString> mStringNames;

    /** String data for a language */
    struct SStringData
    {
        TString String;
        bool IsLocalized;

        SStringData()
            : IsLocalized(false)
        {}

        void Serialize(IArchive& Arc)
        {
            Arc << SerialParameter("String", String)
                << SerialParameter("IsLocalized", IsLocalized, SH_Optional, true);
        }
    };

    struct SLanguageData
    {
        ELanguage Language;
        std::vector<SStringData> Strings;

        void Serialize(IArchive& Arc)
        {
            Arc << SerialParameter("Language", Language)
                << SerialParameter("Strings", Strings);
        }
    };
    std::vector<SLanguageData> mLanguages;

public:
    /** Constructor */
    CStringTable(CResourceEntry *pEntry = 0) : CResource(pEntry) {}

    /** Returns the number of languages in the table */
    inline uint NumLanguages() const    { return mLanguages.size(); }

    /** Returns the number of strings in the table */
    inline uint NumStrings() const      { return mLanguages.empty() ? 0 : mLanguages[0].Strings.size(); }

    /** Returns languages used by index */
    inline ELanguage LanguageByIndex(uint Index) const  { return mLanguages.size() > Index ? mLanguages[Index].Language : ELanguage::Invalid; }

    /** Returns the string name by string index. May be blank if the string at the requested index is unnamed */
    inline TString StringNameByIndex(uint Index) const  { return mStringNames.size() > Index ? mStringNames[Index] : ""; }

    /** Returns a string given a language/index pair */
    TString GetString(ELanguage Language, uint StringIndex) const;

    /** Updates a string for a given language */
    void SetString(ELanguage Language, uint StringIndex, const TString& kNewString);

    /** Updates a string name */
    void SetStringName(uint StringIndex, const TString& kNewName);

    /** Move string to another position in the table */
    void MoveString(uint StringIndex, uint NewIndex);

    /** Add a new string to the table */
    void AddString(uint AtIndex);

    /** Remove a string from the table */
    void RemoveString(uint StringIndex);

    /** Initialize new resource data */
    virtual void InitializeNewResource() override;

    /** Serialize resource data */
    virtual void Serialize(IArchive& Arc) override;

    /** Build the dependency tree for this resource */
    virtual CDependencyTree* BuildDependencyTree() const override;

    /** Static - Strip all formatting tags for a given string */
    static TString StripFormatting(const TString& kInString);

    /** Static - Returns whether a given language is supported by the given game/region combination */
    static bool IsLanguageSupported(ELanguage Language, EGame Game, ERegion Region);
};

#endif // CSTRINGTABLE_H
