#ifndef IPROPERTY
#define IPROPERTY

/* This header file declares some classes used to track script object properties
 * IProperty, TTypedProperty (and typedefs), CPropertyStruct, and CArrayProperty */
#include "EPropertyType.h"
#include "IPropertyValue.h"
#include "Core/Resource/CResource.h"
#include "Core/Resource/TResPtr.h"
#include "Core/Resource/CAnimationParameters.h"
#include <Common/CColor.h>
#include <Common/TString.h>
#include <Math/CVector3f.h>
#include <list>

class CScriptObject;
class CScriptTemplate;
class CStructTemplate;
class IPropertyTemplate;
typedef TString TIDString;

/*
 * IProperty is the base class, containing just some virtual function definitions
 * Virtual destructor is mainly there to make cleanup easy; don't need to cast to delete
 */
class IProperty
{
    friend class CScriptLoader;

protected:
    class CPropertyStruct *mpParent;
    CScriptObject *mpInstance;
    IPropertyTemplate *mpTemplate;

public:
    IProperty(IPropertyTemplate *pTemp, CScriptObject *pInstance, CPropertyStruct *pParent)
        : mpParent(pParent)
        , mpInstance(pInstance)
        , mpTemplate(pTemp)
    {
    }

    virtual ~IProperty() {}
    virtual EPropertyType Type() const = 0;
    virtual TString ToString() const { return ""; }
    virtual IPropertyValue* RawValue() { return nullptr; }
    virtual void Copy(const IProperty *pkProp) = 0;
    virtual IProperty* Clone(CScriptObject *pInstance, CPropertyStruct *pParent = 0) const = 0;
    virtual bool Matches(const IProperty *pkProp) const = 0;

    virtual bool ShouldCook();      // Can't be const because it calls MatchesDefault()
    virtual bool MatchesDefault();  // Can't be const because RawValue() isn't const

    inline CScriptObject* Instance() const { return mpInstance; }
    inline CPropertyStruct* Parent() const { return mpParent; }
    inline void SetParent(CPropertyStruct *pParent) { mpParent = pParent; }

    bool IsInArray() const;
    CPropertyStruct* RootStruct();

    // These functions can't be in the header to avoid circular includes with IPropertyTemplate.h
    IPropertyTemplate* Template() const;
    TString Name() const;
    u32 ID() const;
    TIDString IDString(bool FullPath) const;
    u32 ArrayIndex() const;
};

/*
 * TTypedProperty is a template subclass for actual properties.
 */
template <typename ValueType, EPropertyType TypeEnum, class ValueClass>
class TTypedProperty : public IProperty
{
    friend class CScriptLoader;
    ValueClass mValue;
public:
    TTypedProperty(IPropertyTemplate *pTemp, CScriptObject *pInstance, CPropertyStruct *pParent)
        : IProperty(pTemp, pInstance, pParent) {}

    TTypedProperty(IPropertyTemplate *pTemp, CScriptObject *pInstance, CPropertyStruct *pParent, ValueType v)
        : IProperty(pTemp, pInstance, pParent), mValue(v) {}

    ~TTypedProperty() {}
    virtual EPropertyType Type() const { return TypeEnum; }
    static inline EPropertyType StaticType() { return TypeEnum; }

    virtual TString ToString() const { return mValue.ToString(); }
    virtual IPropertyValue* RawValue() { return &mValue; }

    virtual void Copy(const IProperty *pkProp)
    {
        const TTypedProperty *pkCast = static_cast<const TTypedProperty*>(pkProp);
        mValue.Set(pkCast->mValue.Get());
    }

    virtual TTypedProperty* Clone(class CScriptObject *pInstance, CPropertyStruct *pParent) const
    {
        if (!pParent) pParent = mpParent;

        TTypedProperty *pOut = new TTypedProperty(mpTemplate, pInstance, pParent);
        pOut->Copy(this);
        return pOut;
    }

    virtual bool Matches(const IProperty *pkProp) const
    {
        const TTypedProperty *pkTyped = static_cast<const TTypedProperty*>(pkProp);
        return ( (Type() == pkTyped->Type()) &&
                 mValue.Matches(&pkTyped->mValue) );
    }

    inline ValueType Get() const { return mValue.Get(); }
    inline void Set(ValueType v) { mValue.Set(v); }
};
typedef TTypedProperty<bool, eBoolProperty, CBoolValue>                             TBoolProperty;
typedef TTypedProperty<char, eByteProperty, CByteValue>                             TByteProperty;
typedef TTypedProperty<short, eShortProperty, CShortValue>                          TShortProperty;
typedef TTypedProperty<long, eLongProperty, CLongValue>                             TLongProperty;
typedef TTypedProperty<long, eEnumProperty, CLongValue>                             TEnumProperty;
typedef TTypedProperty<long, eBitfieldProperty, CHexLongValue>                      TBitfieldProperty;
typedef TTypedProperty<float, eFloatProperty, CFloatValue>                          TFloatProperty;
typedef TTypedProperty<CVector3f, eVector3Property, CVector3Value>                  TVector3Property;
typedef TTypedProperty<CColor, eColorProperty, CColorValue>                         TColorProperty;
typedef TTypedProperty<std::vector<u8>, eUnknownProperty, CUnknownValue>            TUnknownProperty;

/*
 * TStringProperty, TFileProperty, and TCharacterProperty get little subclasses in order to override some virtual functions.
 */
#define IMPLEMENT_PROPERTY_CTORS(ClassName, ValueType) \
    ClassName(IPropertyTemplate *pTemp, CScriptObject *pInstance, CPropertyStruct *pParent) \
        : TTypedProperty(pTemp, pInstance, pParent) {} \
    \
    ClassName(IPropertyTemplate *pTemp, CScriptObject *pInstance, CPropertyStruct *pParent, ValueType v) \
        : TTypedProperty(pTemp, pInstance, pParent, v) {}

class TStringProperty : public TTypedProperty<TString, eStringProperty, CStringValue>
{
public:
    IMPLEMENT_PROPERTY_CTORS(TStringProperty, TString)
    virtual bool MatchesDefault()   { return Get().IsEmpty(); }
    virtual bool ShouldCook()       { return true; }
};

class TFileProperty : public TTypedProperty<CResourceInfo, eFileProperty, CFileValue>
{
public:
    IMPLEMENT_PROPERTY_CTORS(TFileProperty, CResourceInfo)
    virtual bool MatchesDefault()   { return !Get().IsValid(); }
    virtual bool ShouldCook()       { return true; }
};

class TCharacterProperty : public TTypedProperty<CAnimationParameters, eCharacterProperty, CCharacterValue>
{
public:
    IMPLEMENT_PROPERTY_CTORS(TCharacterProperty, CAnimationParameters)
    virtual bool MatchesDefault()   { return Get().AnimSet() == nullptr; }
    virtual bool ShouldCook()       { return true; }
};

class TMayaSplineProperty : public TTypedProperty<std::vector<u8>, eMayaSplineProperty, CMayaSplineValue>
{
public:
    IMPLEMENT_PROPERTY_CTORS(TMayaSplineProperty, std::vector<u8>)
    virtual bool MatchesDefault() { return Get().empty(); }
};

/*
 * CPropertyStruct is for defining structs of properties.
 */
class CPropertyStruct : public IProperty
{
    friend class CScriptLoader;
protected:
    std::vector<IProperty*> mProperties;
public:
    CPropertyStruct(IPropertyTemplate *pTemp, CScriptObject *pInstance, CPropertyStruct *pParent)
        : IProperty(pTemp, pInstance, pParent) {}

    ~CPropertyStruct()
    {
        for (auto it = mProperties.begin(); it != mProperties.end(); it++)
            delete *it;
    }

    EPropertyType Type() const { return eStructProperty; }
    static inline EPropertyType StaticType() { return eStructProperty; }

    virtual void Copy(const IProperty *pkProp);

    virtual IProperty* Clone(CScriptObject *pInstance, CPropertyStruct *pParent) const
    {
        if (!pParent) pParent = mpParent;
        CPropertyStruct *pOut = new CPropertyStruct(mpTemplate, pInstance, pParent);
        pOut->Copy(this);
        return pOut;
    }

    virtual bool Matches(const IProperty *pkProp) const
    {
        const CPropertyStruct *pkStruct = static_cast<const CPropertyStruct*>(pkProp);

        if ( (Type() == pkStruct->Type()) &&
             (mProperties.size() == pkStruct->mProperties.size()) )
        {
            for (u32 iProp = 0; iProp < mProperties.size(); iProp++)
            {
                if (!mProperties[iProp]->Matches(pkStruct->mProperties[iProp]))
                    return false;
            }

            return true;
        }

        return false;
    }

    virtual bool MatchesDefault()
    {
        for (u32 iProp = 0; iProp < mProperties.size(); iProp++)
        {
            if (!mProperties[iProp]->MatchesDefault())
                return false;
        }

        return true;
    }

    virtual bool ShouldCook();

    // Inline
    inline u32 Count() const { return mProperties.size(); }
    inline void AddSubProperty(IProperty *pProp) { mProperties.push_back(pProp); }
    inline IProperty* operator[](u32 index) { return mProperties[index]; }

    // Functions
    IProperty* PropertyByIndex(u32 index) const;
    IProperty* PropertyByID(u32 ID) const;
    IProperty* PropertyByIDString(const TIDString& rkStr) const;
    CPropertyStruct* StructByIndex(u32 index) const;
    CPropertyStruct* StructByID(u32 ID) const;
    CPropertyStruct* StructByIDString(const TIDString& rkStr) const;
};

/*
 * CArrayProperty stores a repeated property struct.
 */
class CArrayProperty : public CPropertyStruct
{
    friend class CScriptLoader;

public:
    CArrayProperty(IPropertyTemplate *pTemp, CScriptObject *pInstance, CPropertyStruct *pParent)
        : CPropertyStruct(pTemp, pInstance, pParent) {}

    EPropertyType Type() const { return eArrayProperty; }
    static inline EPropertyType StaticType() { return eArrayProperty; }

    virtual IProperty* Clone(CScriptObject *pInstance, CPropertyStruct *pParent) const
    {
        if (!pParent) pParent = mpParent;
        CArrayProperty *pOut = new CArrayProperty(mpTemplate, pInstance, pParent);
        pOut->Copy(this);
        return pOut;
    }

    virtual bool MatchesDefault() { return mProperties.empty(); }
    virtual bool ShouldCook();

    // Inline
    inline void Reserve(u32 amount) { mProperties.reserve(amount); }

    // Functions
    void Resize(int Size);
    CStructTemplate* SubStructTemplate() const;
    TString ElementName() const;
};

/*
 * Function for casting properties. Returns null if the property is not actually the requested type.
 */
template <class PropertyClass>
PropertyClass* TPropCast(IProperty *pProp)
{
    return (pProp && pProp->Type() == PropertyClass::StaticType() ? static_cast<PropertyClass*>(pProp) : nullptr);
}

#endif // IPROPERTY

