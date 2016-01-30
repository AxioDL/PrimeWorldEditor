#ifndef IPROPERTYTEMPLATE
#define IPROPERTYTEMPLATE

#include "EPropertyType.h"
#include "IProperty.h"
#include "IPropertyValue.h"
#include "Core/Resource/CAnimationParameters.h"
#include <Common/CColor.h>
#include <Common/TString.h>
#include <Common/types.h>
#include <Math/CVector3f.h>
#include <vector>

typedef TString TIDString;
class CStructTemplate;
class IProperty;

enum ECookPreference
{
    eNoCookPreference,
    eAlwaysCook,
    eNeverCook
};

// IPropertyTemplate - Base class. Contains basic info that every property has,
// plus virtual functions for determining more specific property type.
class IPropertyTemplate
{
    friend class CTemplateLoader;
    friend class CTemplateWriter;

protected:
    CStructTemplate *mpParent;
    TString mName;
    TString mDescription;
    u32 mID;
    ECookPreference mCookPreference;
    std::vector<u32> mAllowedVersions;

public:
    IPropertyTemplate(u32 ID, CStructTemplate *pParent = 0)
        : mID(ID)
        , mpParent(pParent)
        , mName("Unknown")
        , mCookPreference(eNoCookPreference)
    {
    }

    IPropertyTemplate(u32 ID, const TString& rkName, ECookPreference CookPreference, CStructTemplate *pParent = 0)
        : mID(ID)
        , mpParent(pParent)
        , mName(rkName)
        , mCookPreference(CookPreference)
    {
    }

    virtual EPropertyType Type()  const = 0;
    virtual bool CanHaveDefault() const = 0;
    virtual bool IsNumerical()    const = 0;

    virtual bool HasValidRange() const  { return false; }
    virtual TString DefaultToString()   { return ""; }
    virtual TString RangeToString()     { return ""; }

    virtual void SetParam(const TString& rkParamName, const TString& rkValue)
    {
        if (rkParamName == "should_cook")
        {
            TString lValue = rkValue.ToLower();

            if (lValue == "always")
                mCookPreference = eAlwaysCook;
            else if (lValue == "never")
                mCookPreference = eNeverCook;
            else
                mCookPreference = eNoCookPreference;
        }

        else if (rkParamName == "description")
            mDescription = rkValue;
    }

    virtual IProperty* InstantiateProperty(CPropertyStruct *pParent) = 0;

    inline TString Name() const
    {
        return mName;
    }

    inline TString Description() const
    {
        return mDescription;
    }

    inline u32 PropertyID() const
    {
        return mID;
    }

    inline ECookPreference CookPreference() const
    {
        return mCookPreference;
    }

    inline void SetName(const TString& rkName)
    {
        mName = rkName;
    }

    inline void SetDescription(const TString& rkDesc)
    {
        mDescription = rkDesc;
    }

    inline CStructTemplate* Parent() const
    {
        return mpParent;
    }

    bool IsInVersion(u32 Version) const;
    TIDString IDString(bool FullPath) const;
    CStructTemplate* RootStruct();
};

// TTypedPropertyTemplate - Template property class that allows for tracking
// a default value. Typedefs are set up for a bunch of property types.
template<typename PropType, EPropertyType PropTypeEnum, class ValueClass>
class TTypedPropertyTemplate : public IPropertyTemplate
{
    friend class CTemplateLoader;
    friend class CTemplateWriter;

protected:
    ValueClass mDefaultValue;

public:
    TTypedPropertyTemplate(u32 ID, CStructTemplate *pParent = 0)
        : IPropertyTemplate(ID, pParent) {}

    TTypedPropertyTemplate(u32 ID, const TString& rkName, ECookPreference CookPreference, CStructTemplate *pParent = 0)
        : IPropertyTemplate(ID, rkName, CookPreference, pParent) {}

    virtual EPropertyType Type()  const { return PropTypeEnum; }
    virtual bool CanHaveDefault() const { return true;         }
    virtual bool IsNumerical()    const { return false;        }

    virtual TString DefaultToString()
    {
        return mDefaultValue.ToString();
    }

    virtual void SetParam(const TString& rkParamName, const TString& rkValue)
    {
        IPropertyTemplate::SetParam(rkParamName, rkValue);

        if (rkParamName == "default")
            mDefaultValue.FromString(rkValue.ToLower());
    }

    virtual IProperty* InstantiateProperty(CPropertyStruct *pParent)
    {
        typedef TTypedProperty<PropType, PropTypeEnum, ValueClass> TPropertyType;

        TPropertyType *pOut = new TPropertyType(this, pParent);
        pOut->Set(GetDefaultValue());
        return pOut;
    }

    inline PropType GetDefaultValue() const
    {
        return mDefaultValue.Get();
    }

    inline void SetDefaultValue(const PropType& rkIn)
    {
        mDefaultValue.Set(rkIn);
    }
};

// TNumericalPropertyTemplate - Subclass of TTypedPropertyTemplate for numerical
// property types, and allows a min/max value to be tracked.
template<typename PropType, EPropertyType PropTypeEnum, class ValueClass>
class TNumericalPropertyTemplate : public TTypedPropertyTemplate<PropType,PropTypeEnum,ValueClass>
{
    friend class CTemplateLoader;
    friend class CTemplateWriter;

    ValueClass mMin;
    ValueClass mMax;

public:
    TNumericalPropertyTemplate(u32 ID, CStructTemplate *pParent = 0)
        : TTypedPropertyTemplate(ID, pParent)
    {}

    TNumericalPropertyTemplate(u32 ID, const TString& rkName, ECookPreference CookPreference, CStructTemplate *pParent = 0)
        : TTypedPropertyTemplate(ID, rkName, CookPreference, pParent)
        , mMin(0)
        , mMax(0)
    {}

    virtual bool IsNumerical() const { return true; }

    virtual bool HasValidRange() const
    {
        return (mMin != 0 || mMax != 0);
    }

    virtual TString RangeToString() const
    {
        return mMin.ToString() + "," + mMax.ToString();
    }

    virtual void SetParam(const TString& rkParamName, const TString& rkValue)
    {
        TTypedPropertyTemplate<PropType,PropTypeEnum,ValueClass>::SetParam(rkParamName, rkValue);

        if (rkParamName == "range")
        {
            TStringList Components = rkValue.ToLower().Split(", ");

            if (Components.size() == 2)
            {
                mMin.FromString(Components.front());
                mMax.FromString(Components.back());
            }
        }
    }

    inline PropType GetMin()
    {
        return mMin.Get();
    }

    inline PropType GetMax()
    {
        return mMax.Get();
    }

    inline void SetRange(const PropType& rkMin, const PropType& rkMax)
    {
        mMin.Set(rkMin);
        mMax.Set(rkMax);
    }
};

// Typedefs for all property types that don't need further functionality.
typedef TTypedPropertyTemplate<bool, eBoolProperty, CBoolValue>             TBoolTemplate;
typedef TNumericalPropertyTemplate<s8, eByteProperty, CByteValue>           TByteTemplate;
typedef TNumericalPropertyTemplate<s16, eShortProperty, CShortValue>        TShortTemplate;
typedef TNumericalPropertyTemplate<s32, eLongProperty, CLongValue>          TLongTemplate;
typedef TNumericalPropertyTemplate<float, eFloatProperty, CFloatValue>      TFloatTemplate;
typedef TTypedPropertyTemplate<TString, eStringProperty, CStringValue>      TStringTemplate;
typedef TTypedPropertyTemplate<CVector3f, eVector3Property, CVector3Value>  TVector3Template;
typedef TTypedPropertyTemplate<CColor, eColorProperty, CColorValue>         TColorTemplate;

// CFileTemplate - Property template for files. Tracks a list of file types that
// the property is allowed to accept.
class CFileTemplate : public IPropertyTemplate
{
    friend class CTemplateLoader;
    friend class CTemplateWriter;

    TStringList mAcceptedExtensions;
public:
    CFileTemplate(u32 ID, CStructTemplate *pParent = 0)
        : IPropertyTemplate(ID, pParent) {}

    CFileTemplate(u32 ID, const TString& rkName, ECookPreference CookPreference, CStructTemplate *pParent = 0)
        : IPropertyTemplate(ID, rkName, CookPreference, pParent) {}

    virtual EPropertyType Type() const  { return eFileProperty; }
    virtual bool CanHaveDefault() const { return false; }
    virtual bool IsNumerical() const    { return false; }

    IProperty* InstantiateProperty(CPropertyStruct *pParent)
    {
        return new TFileProperty(this, pParent);
    }

    void SetAllowedExtensions(const TStringList& rkExtensions)
    {
        mAcceptedExtensions = rkExtensions;
    }

    bool AcceptsExtension(const TString& rkExtension)
    {
        for (auto it = mAcceptedExtensions.begin(); it != mAcceptedExtensions.end(); it++)
            if (*it == rkExtension) return true;
        return false;
    }

    const TStringList& Extensions() const
    {
        return mAcceptedExtensions;
    }
};

// CCharacterTemplate - Typed property that doesn't allow default values.
class CCharacterTemplate : public TTypedPropertyTemplate<CAnimationParameters,
                                                         eCharacterProperty,
                                                         CCharacterValue>
{
    friend class CTemplateLoader;
    friend class CTemplateWriter;

public:
    CCharacterTemplate(u32 ID, CStructTemplate *pParent = 0)
        : TTypedPropertyTemplate(ID, pParent) { }

    CCharacterTemplate(u32 ID, const TString& rkName, ECookPreference CookPreference, CStructTemplate *pParent = 0)
        : TTypedPropertyTemplate(ID, rkName, CookPreference, pParent) { }

    virtual bool CanHaveDefault() const
    {
        return false;
    }
};

// CEnumTemplate - Property template for enums. Tracks a list of possible values (enumerators).
class CEnumTemplate : public TLongTemplate
{
    friend class CTemplateLoader;
    friend class CTemplateWriter;

    struct SEnumerator
    {
        TString Name;
        u32 ID;

        SEnumerator(const TString& rkName, u32 _ID)
            : Name(rkName), ID(_ID) {}
    };
    std::vector<SEnumerator> mEnumerators;
    TString mSourceFile;

public:
    CEnumTemplate(u32 ID, CStructTemplate *pParent = 0)
        : TLongTemplate(ID, pParent)
    {
        mDefaultValue.SetHexStringOutput(true);
    }

    CEnumTemplate(u32 ID, const TString& rkName, ECookPreference CookPreference, CStructTemplate *pParent = 0)
        : TLongTemplate(ID, rkName, CookPreference, pParent)
    {
        mDefaultValue.SetHexStringOutput(true);
    }

    virtual EPropertyType Type()  const { return eEnumProperty; }
    virtual bool CanHaveDefault() const { return true; }
    virtual bool IsNumerical()    const { return false; }

    virtual IProperty* InstantiateProperty(CPropertyStruct *pParent)
    {
        TEnumProperty *pEnum = new TEnumProperty(this, pParent);
        u32 Index = EnumeratorIndex(GetDefaultValue());
        pEnum->Set(Index);
        return pEnum;
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

// CBitfieldTemplate - Property template for bitfields, which can have multiple
// distinct boolean parameters packed into one property.
class CBitfieldTemplate : public TLongTemplate
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
    CBitfieldTemplate(u32 ID, CStructTemplate *pParent = 0)
        : TLongTemplate(ID, pParent)
    {
        mDefaultValue.SetHexStringOutput(true);
    }

    CBitfieldTemplate(u32 ID, const TString& rkName, ECookPreference CookPreference, CStructTemplate *pParent = 0)
        : TLongTemplate(ID, rkName, CookPreference, pParent)
    {
        mDefaultValue.SetHexStringOutput(true);
    }

    virtual EPropertyType Type()  const { return eBitfieldProperty; }
    virtual bool CanHaveDefault() const { return true; }
    virtual bool IsNumerical()    const { return false; }

    virtual IProperty* InstantiateProperty(CPropertyStruct *pParent)
    {
        TBitfieldProperty *pBitfield = new TBitfieldProperty(this, pParent);
        pBitfield->Set(GetDefaultValue());
        return pBitfield;
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

// CStructTemplate - Defines structs composed of multiple sub-properties.
class CStructTemplate : public IPropertyTemplate
{
    friend class CTemplateLoader;
    friend class CTemplateWriter;

protected:
    std::vector<IPropertyTemplate*> mSubProperties;
    std::vector<u32> mVersionPropertyCounts;
    bool mIsSingleProperty;
    TString mSourceFile;

    void DetermineVersionPropertyCounts();
public:
    CStructTemplate(u32 ID, CStructTemplate *pParent = 0)
        : IPropertyTemplate(ID, pParent)
        , mIsSingleProperty(false) {}

    CStructTemplate(u32 ID, const TString& rkName, ECookPreference CookPreference, CStructTemplate *pParent = 0)
        : IPropertyTemplate(ID, rkName, CookPreference, pParent)
        , mIsSingleProperty(false) {}

    ~CStructTemplate()
    {
        for (auto it = mSubProperties.begin(); it != mSubProperties.end(); it++)
            delete *it;
    }

    EPropertyType Type()  const { return eStructProperty; }
    bool CanHaveDefault() const { return false; }
    bool IsNumerical()    const { return false; }

    IProperty* InstantiateProperty(CPropertyStruct *pParent)
    {
        CPropertyStruct *pStruct = new CPropertyStruct(this, pParent);

        for (u32 iSub = 0; iSub < mSubProperties.size(); iSub++)
        {
            IProperty *pSubProp = mSubProperties[iSub]->InstantiateProperty(pStruct);
            pStruct->AddSubProperty(pSubProp);
        }

        return pStruct;
    }

    bool IsSingleProperty() const;
    u32 Count() const;
    u32 NumVersions();
    u32 PropertyCountForVersion(u32 Version);
    u32 VersionForPropertyCount(u32 PropCount);
    IPropertyTemplate* PropertyByIndex(u32 index);
    IPropertyTemplate* PropertyByID(u32 ID);
    IPropertyTemplate* PropertyByIDString(const TIDString& str);
    CStructTemplate* StructByIndex(u32 index);
    CStructTemplate* StructByID(u32 ID);
    CStructTemplate* StructByIDString(const TIDString& str);
    bool HasProperty(const TIDString& rkIdString);
    void DebugPrintProperties(TString base);
};

// CArrayTemplate - Defines a repeating struct composed of multiple sub-properties.
// Similar to CStructTemplate, but with new implementations of Type() and InstantiateProperty().
class CArrayTemplate : public CStructTemplate
{
    friend class CTemplateLoader;
    friend class CTemplateWriter;
    TString mElementName;

public:
    CArrayTemplate(u32 ID, CStructTemplate *pParent = 0)
        : CStructTemplate(ID, pParent)
    {
        mIsSingleProperty = true;
    }

    CArrayTemplate(u32 ID, const TString& rkName, ECookPreference CookPreference, CStructTemplate *pParent = 0)
        : CStructTemplate(ID, rkName, CookPreference, pParent)
    {
        mIsSingleProperty = true;
    }

    EPropertyType Type() const { return eArrayProperty; }

    void SetParam(const TString& rkParamName, const TString& rkValue)
    {
        if (rkParamName == "element_name")
            mElementName = rkValue;
        else
            CStructTemplate::SetParam(rkParamName, rkValue);
    }

    IProperty* InstantiateProperty(CPropertyStruct *pParent)
    {
        return new CArrayProperty(this, pParent);
    }

    TString ElementName() const                { return mElementName; }
    void SetElementName(const TString& rkName) { mElementName = rkName; }

    CPropertyStruct* CreateSubStruct(CArrayProperty *pArray)
    {
        return (CPropertyStruct*) CStructTemplate::InstantiateProperty(pArray);
    }
};

#endif // IPROPERTYTEMPLATE

