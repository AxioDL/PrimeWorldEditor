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

/** TPropertyRef: Embeds a reference to a property on a specific script object */
template<class PropertyClass, typename ValueType = PropertyClass::ValueType>
class TPropertyRef
{
    /** Script object containing the property data being referenced */
    CScriptObject* mpObject;

    /** Property being referenced */
    PropertyClass* mpProperty;

public:
    TPropertyRef()
        : mpObject(nullptr), mpProperty(nullptr)
    {}

    TPropertyRef(CScriptObject* pInObject, IPropertyNew* pInProperty)
        : mpObject(pInObject), mpProperty( TPropCast<PropertyClass>(pInProperty) )
    {
    }

    TPropertyRef(CScriptObject* pInObject, PropertyClass* pInProperty)
        : mpObject(pInObject), mpProperty(pInProperty)
    {
    }

    /** Accessors */
    inline CScriptObject* Object() const        { return mpObject; }
    inline PropertyClass* Property() const      { return mpProperty; }
    inline ValueType Get() const                { return IsValid() ? *((ValueType*) mpProperty->RawValuePtr( mpObject->PropertyData() )) : ValueType(); }
    inline void Set(const ValueType& kIn) const { if (IsValid()) *((ValueType*) mpProperty->RawValuePtr( mpObject->PropertyData() )) = kIn; }
    inline bool IsValid() const                 { return mpObject != nullptr && mpProperty != nullptr; }

    /** Inline operators */
    inline operator ValueType() const
    {
        return Get();
    }

    inline bool operator==(IPropertyNew* pProperty) const
    {
        return mpProperty == pProperty;
    }

    friend bool operator==(IPropertyNew* pLeft, const TPropertyRef& kRight)
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
typedef TPropertyRef<CStructPropertyNew>    CStructRef;
typedef TPropertyRef<CArrayProperty>        CArrayRef;

/** Special version for enums */
template<typename ValueType>
class TEnumRef : public TPropertyRef<CEnumProperty, ValueType>
{
public:
    TEnumRef()
        : TPropertyRef()
    {}

    TEnumRef(CScriptObject* pInObject, IPropertyNew* pInProperty)
        : TPropertyRef(pInObject, pInProperty)
    {}

    TEnumRef(CScriptObject* pInObject, CEnumProperty* pInProperty)
        : TPropertyRef(pInObject, pInProperty)
    {}
};

#endif // TPROPERTYREF_H
