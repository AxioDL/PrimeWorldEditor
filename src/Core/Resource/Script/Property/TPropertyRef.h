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
template<class PropertyClass, typename ValueType = typename PropertyClass::ValueType>
class TPropertyRef
{
    /** Property data being referenced */
    void*                   mpPropertyData = nullptr;

    /** Property being referenced */
    PropertyClass*          mpProperty = nullptr;

public:
    TPropertyRef() = default;

    explicit TPropertyRef(void* pInData, IProperty* pInProperty)
        : mpPropertyData(pInData), mpProperty( TPropCast<PropertyClass>(pInProperty) )
    {
    }

    explicit TPropertyRef(void* pInData, PropertyClass* pInProperty)
        : mpPropertyData(pInData), mpProperty(pInProperty)
    {
    }

    /** Accessors */
    void* DataPointer() const            { return mpPropertyData; }
    PropertyClass* Property() const      { return mpProperty; }
    ValueType Get() const                { return IsValid() ? *((ValueType*) mpProperty->RawValuePtr( mpPropertyData )) : ValueType(); }
    void Set(const ValueType& kIn) const { if (IsValid()) *((ValueType*) mpProperty->RawValuePtr( mpPropertyData )) = kIn; }
    bool IsValid() const                 { return mpPropertyData != nullptr && mpProperty != nullptr; }

    /** Inline operators */
    operator ValueType() const
    {
        return Get();
    }

    bool operator==(const IProperty* pProperty) const
    {
        return mpProperty == pProperty;
    }
    bool operator!=(const IProperty* pProperty) const
    {
        return !operator==(pProperty);
    }

    friend bool operator==(const IProperty* pLeft, const TPropertyRef& kRight)
    {
        return pLeft == kRight.Property();
    }
    friend bool operator!=(const IProperty* pLeft, const TPropertyRef& kRight)
    {
        return !(*pLeft == *kRight);
    }

    friend bool operator==(const TPropertyRef& left, const IProperty* right)
    {
        return left.Property() == right;
    }
    friend bool operator!=(const TPropertyRef& left, const IProperty* right)
    {
        return !(left.Property() == right);
    }
};

/** Convenience typedefs */
using CAnimationRef = TPropertyRef<CAnimationProperty>;
using CAnimationSetRef = TPropertyRef<CAnimationSetProperty>;
using CArrayRef = TPropertyRef<CArrayProperty>;
using CAssetRef = TPropertyRef<CAssetProperty>;
using CBoolRef = TPropertyRef<CBoolProperty>;
using CByteRef = TPropertyRef<CByteProperty>;
using CColorRef = TPropertyRef<CColorProperty>;
using CFlagsRef = TPropertyRef<CFlagsProperty>;
using CFloatRef = TPropertyRef<CFloatProperty>;
using CGuidRef = TPropertyRef<CGuidProperty>;
using CIntRef = TPropertyRef<CIntProperty>;
using CPointerRef = TPropertyRef<CPointerProperty>;
using CSequenceRef = TPropertyRef<CSequenceProperty>;
using CShortRef = TPropertyRef<CShortProperty>;
using CSoundRef = TPropertyRef<CSoundProperty>;
using CSplineRef = TPropertyRef<CSplineProperty>;
using CStringRef = TPropertyRef<CStringProperty>;
using CStructRef = TPropertyRef<CStructProperty>;
using CVectorRef = TPropertyRef<CVectorProperty>;

/** Special version for enums */
template<typename ValueType>
class TEnumRef : public TPropertyRef<CEnumProperty, ValueType>
{
    using base = TPropertyRef<CEnumProperty, ValueType>;
public:
    TEnumRef()
        : base()
    {}

    TEnumRef(void* pInData, IProperty* pInProperty)
        : base(pInData, pInProperty)
    {}

    TEnumRef(void* pInData, CEnumProperty* pInProperty)
        : base(pInData, pInProperty)
    {}
};

#endif // TPROPERTYREF_H
