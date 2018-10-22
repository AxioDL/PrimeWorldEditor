#ifndef TPROPERTYREF_H
#define TPROPERTYREF_H

#include "CAnimationProperty.h"
#include "CAnimationSetProperty.h"
#include "CArrayProperty.h"
#include "CAssetProperty.h"
#include "CBoolProperty.h"
#include "CByteProperty.h"
#include "CColorProperty.h"
#include "CEnumProperty.h"
#include "CFlagsProperty.h"
#include "CFloatProperty.h"
#include "CGuidProperty.h"
#include "CIntProperty.h"
#include "CPointerProperty.h"
#include "CSequenceProperty.h"
#include "CShortProperty.h"
#include "CSoundProperty.h"
#include "CSplineProperty.h"
#include "CStringProperty.h"
#include "CStructProperty.h"
#include "CVectorProperty.h"

/** TPropertyRef: Embeds a reference to a property on a specific object */
template<class PropertyClass, typename ValueType = PropertyClass::ValueType>
class TPropertyRef
{
    /** Property data being referenced */
    void*                   mpPropertyData;

    /** Property being referenced */
    PropertyClass*          mpProperty;

public:
    TPropertyRef()
        : mpPropertyData(nullptr), mpProperty(nullptr)
    {}

    explicit TPropertyRef(void* pInData, IProperty* pInProperty)
        : mpPropertyData(pInData), mpProperty( TPropCast<PropertyClass>(pInProperty) )
    {
    }

    explicit TPropertyRef(void* pInData, PropertyClass* pInProperty)
        : mpPropertyData(pInData), mpProperty(pInProperty)
    {
    }

    /** Accessors */
    inline void* DataPointer() const            { return mpPropertyData; }
    inline PropertyClass* Property() const      { return mpProperty; }
    inline ValueType Get() const                { return IsValid() ? *((ValueType*) mpProperty->RawValuePtr( mpPropertyData )) : ValueType(); }
    inline void Set(const ValueType& kIn) const { if (IsValid()) *((ValueType*) mpProperty->RawValuePtr( mpPropertyData )) = kIn; }
    inline bool IsValid() const                 { return mpPropertyData != nullptr && mpProperty != nullptr; }

    /** Inline operators */
    inline operator ValueType() const
    {
        return Get();
    }

    inline bool operator==(IProperty* pProperty) const
    {
        return mpProperty == pProperty;
    }

    friend bool operator==(IProperty* pLeft, const TPropertyRef& kRight)
    {
        return pLeft == kRight.Property();
    }
};

/** Convenience typedefs */
typedef TPropertyRef<CBoolProperty>         CBoolRef;
typedef TPropertyRef<CByteProperty>         CByteRef;
typedef TPropertyRef<CShortProperty>        CShortRef;
typedef TPropertyRef<CIntProperty>          CIntRef;
typedef TPropertyRef<CFloatProperty>        CFloatRef;
typedef TPropertyRef<CFlagsProperty>        CFlagsRef;
typedef TPropertyRef<CStringProperty>       CStringRef;
typedef TPropertyRef<CVectorProperty>       CVectorRef;
typedef TPropertyRef<CColorProperty>        CColorRef;
typedef TPropertyRef<CAssetProperty>        CAssetRef;
typedef TPropertyRef<CSoundProperty>        CSoundRef;
typedef TPropertyRef<CAnimationProperty>    CAnimationRef;
typedef TPropertyRef<CAnimationSetProperty> CAnimationSetRef;
typedef TPropertyRef<CSequenceProperty>     CSequenceRef;
typedef TPropertyRef<CSplineProperty>       CSplineRef;
typedef TPropertyRef<CGuidProperty>         CGuidRef;
typedef TPropertyRef<CPointerProperty>      CPointerRef;
typedef TPropertyRef<CStructProperty>       CStructRef;
typedef TPropertyRef<CArrayProperty>        CArrayRef;

/** Special version for enums */
template<typename ValueType>
class TEnumRef : public TPropertyRef<CEnumProperty, ValueType>
{
public:
    TEnumRef()
        : TPropertyRef()
    {}

    TEnumRef(void* pInData, IProperty* pInProperty)
        : TPropertyRef(pInData, pInProperty)
    {}

    TEnumRef(void* pInData, CEnumProperty* pInProperty)
        : TPropertyRef(pInData, pInProperty)
    {}
};

#endif // TPROPERTYREF_H
