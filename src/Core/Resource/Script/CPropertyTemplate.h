#ifndef CPROPERTYTEMPLATE
#define CPROPERTYTEMPLATE

#include "EPropertyType.h"
#include <Common/TString.h>
#include <Common/types.h>
#include <vector>

typedef TString TIDString;

class CPropertyTemplate
{
    friend class CTemplateLoader;
    friend class CTemplateWriter;

protected:
    EPropertyType mPropType;
    TString mPropName;
    u32 mPropID;
public:
    CPropertyTemplate(u32 ID)
        : mPropID(ID)
    {
    }

    CPropertyTemplate(EPropertyType type, TString name, u32 ID)
        : mPropType(type),
          mPropName(name),
          mPropID(ID)
    {
    }

    virtual EPropertyType Type() const
    {
        return mPropType;
    }

    inline TString Name() const
    {
        return mPropName;
    }

    inline u32 PropertyID() const
    {
        return mPropID;
    }

    inline void SetName(const TString& Name)
    {
        mPropName = Name;
    }
};

class CFileTemplate : public CPropertyTemplate
{
    friend class CTemplateLoader;
    friend class CTemplateWriter;

    TStringList mAcceptedExtensions;
public:
    CFileTemplate(u32 ID) : CPropertyTemplate(ID) { mPropType = eFileProperty; }

    CFileTemplate(const TString& name, u32 ID, const TStringList& extensions)
        : CPropertyTemplate(ID)
    {
        mPropType = eFileProperty;
        mPropName = name;
        mAcceptedExtensions = extensions;
    }

    EPropertyType Type() const
    {
        return eFileProperty;
    }

    const TStringList& Extensions() const
    {
        return mAcceptedExtensions;
    }
};

class CEnumTemplate : public CPropertyTemplate
{
    friend class CTemplateLoader;
    friend class CTemplateWriter;

    struct SEnumerator
    {
        TString Name;
        u32 ID;

        SEnumerator(const TString& _name, u32 _ID)
            : Name(_name), ID(_ID) {}
    };
    std::vector<SEnumerator> mEnumerators;
    TString mSourceFile;

public:
    CEnumTemplate(u32 ID)
        : CPropertyTemplate(ID)
    {
        mPropType = eEnumProperty;
    }

    CEnumTemplate(const TString& name, u32 ID)
        : CPropertyTemplate(eEnumProperty, name, ID)
    {}

    EPropertyType Type() const
    {
        return eEnumProperty;
    }

    u32 NumEnumerators()
    {
        return mEnumerators.size();
    }

    u32 EnumeratorIndex(u32 enumID)
    {
        for (u32 iEnum = 0; iEnum < mEnumerators.size(); iEnum++)
        {
            if (mEnumerators[iEnum].ID == enumID)
                return iEnum;
        }
        return -1;
    }

    u32 EnumeratorID(u32 enumIndex)
    {
        if (mEnumerators.size() > enumIndex)
            return mEnumerators[enumIndex].ID;

        else return -1;
    }

    TString EnumeratorName(u32 enumIndex)
    {
        if (mEnumerators.size() > enumIndex)
            return mEnumerators[enumIndex].Name;

        else return "INVALID ENUM INDEX";
    }
};

class CBitfieldTemplate : public CPropertyTemplate
{
    friend class CTemplateLoader;
    friend class CTemplateWriter;

    struct SBitFlag
    {
        TString Name;
        u32 Mask;

        SBitFlag(const TString& _name, u32 _mask)
            : Name(_name), Mask(_mask) {}
    };
    std::vector<SBitFlag> mBitFlags;
    TString mSourceFile;

public:
    CBitfieldTemplate(u32 ID)
        : CPropertyTemplate(ID)
    {
        mPropType = eBitfieldProperty;
    }

    CBitfieldTemplate(const TString& name, u32 ID)
        : CPropertyTemplate(eBitfieldProperty, name, ID)
    {}

    EPropertyType Type() const
    {
        return eBitfieldProperty;
    }

    u32 NumFlags()
    {
        return mBitFlags.size();
    }

    TString FlagName(u32 index)
    {
        return mBitFlags[index].Name;
    }

    u32 FlagMask(u32 index)
    {
        return mBitFlags[index].Mask;
    }
};

class CStructTemplate : public CPropertyTemplate
{
    friend class CTemplateLoader;
    friend class CTemplateWriter;

    bool mIsSingleProperty;
    std::vector<CPropertyTemplate*> mProperties;
    TString mSourceFile;
public:
    CStructTemplate();
    ~CStructTemplate();

    EPropertyType Type() const;
    bool IsSingleProperty() const;
    u32 Count() const;
    CPropertyTemplate* PropertyByIndex(u32 index);
    CPropertyTemplate* PropertyByID(u32 ID);
    CPropertyTemplate* PropertyByIDString(const TIDString& str);
    CStructTemplate* StructByIndex(u32 index);
    CStructTemplate* StructByID(u32 ID);
    CStructTemplate* StructByIDString(const TIDString& str);
   void DebugPrintProperties(TString base);
};

#endif // CPROPERTYTEMPLATE

