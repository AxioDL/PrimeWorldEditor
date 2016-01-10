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
    IPropertyTemplate *mpTemplate;
public:
    IProperty(IPropertyTemplate *pTemp) : mpTemplate(pTemp) {}
    virtual ~IProperty() {}
    virtual EPropertyType Type() = 0;

    IPropertyTemplate* Template();
    TString Name();
    u32 ID();
};

/*
 * TTypedProperty is a template subclass for actual properties.
 */
template <typename PropType, EPropertyType TypeEnum, class ValueClass>
class TTypedProperty : public IProperty
{
    friend class CScriptLoader;
    ValueClass mValue;
public:
    TTypedProperty(IPropertyTemplate *pTemp)
        : IProperty(pTemp) {}

    TTypedProperty(IPropertyTemplate *pTemp, PropType v)
        : IProperty(pTemp), mValue(v) {}

    ~TTypedProperty() {}
    inline EPropertyType Type() { return TypeEnum; }
    inline PropType Get() { return mValue.Get(); }
    inline void Set(PropType v) { mValue.Set(v); }
};
typedef TTypedProperty<bool, eBoolProperty, CBoolValue>                             TBoolProperty;
typedef TTypedProperty<char, eByteProperty, CByteValue>                             TByteProperty;
typedef TTypedProperty<short, eShortProperty, CShortValue>                          TShortProperty;
typedef TTypedProperty<long, eLongProperty, CLongValue>                             TLongProperty;
typedef TTypedProperty<long, eEnumProperty, CLongValue>                             TEnumProperty;
typedef TTypedProperty<long, eBitfieldProperty, CLongValue>                         TBitfieldProperty;
typedef TTypedProperty<float, eFloatProperty, CFloatValue>                          TFloatProperty;
typedef TTypedProperty<TString, eStringProperty, CStringValue>                      TStringProperty;
typedef TTypedProperty<CVector3f, eVector3Property, CVector3Value>                  TVector3Property;
typedef TTypedProperty<CColor, eColorProperty, CColorValue>                         TColorProperty;
typedef TTypedProperty<CResourceInfo, eFileProperty, CFileValue>                    TFileProperty;
typedef TTypedProperty<CAnimationParameters, eCharacterProperty, CCharacterValue>   TAnimParamsProperty;
typedef TTypedProperty<std::vector<u8>, eUnknownProperty, CUnknownValue>            TUnknownProperty;

/*
 * CPropertyStruct is for defining structs of properties.
 */
class CPropertyStruct : public IProperty
{
    friend class CScriptLoader;
    std::vector<IProperty*> mProperties;
public:
    CPropertyStruct(IPropertyTemplate *pTemp)
        : IProperty(pTemp) {}

    ~CPropertyStruct();

    EPropertyType Type() { return eStructProperty; }

    // Inline
    inline u32 Count() { return mProperties.size(); }
    inline void AddSubProperty(IProperty *pProp) { mProperties.push_back(pProp); }
    inline IProperty* operator[](u32 index) { return mProperties[index]; }

    // Functions
    IProperty* PropertyByIndex(u32 index);
    IProperty* PropertyByID(u32 ID);
    IProperty* PropertyByIDString(const TIDString& rkStr);
    CPropertyStruct* StructByIndex(u32 index);
    CPropertyStruct* StructByID(u32 ID);
    CPropertyStruct* StructByIDString(const TIDString& rkStr);
};

/*
 * CArrayProperty stores a repeated property struct.
 */
class CArrayProperty : public IProperty
{
    friend class CScriptLoader;
    std::vector<CPropertyStruct*> mSubStructs;

public:
    CArrayProperty(IPropertyTemplate *pTemp)
        : IProperty(pTemp) {}

    EPropertyType Type() { return eArrayProperty; }

    // Inline
    inline u32 Count() { return mSubStructs.size(); }
    inline void Reserve(u32 amount) { mSubStructs.reserve(amount); }
    inline CPropertyStruct* ElementByIndex(u32 index) { return mSubStructs[index]; }
    inline CPropertyStruct* operator[](u32 index) { return ElementByIndex(index); }

    // Functions
    void Resize(u32 Size);
    CStructTemplate* SubStructTemplate();
};

#endif // IPROPERTY

