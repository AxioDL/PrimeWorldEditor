#ifndef EPROPERTYTYPE
#define EPROPERTYTYPE

#include "IPropertyNew.h"

#if 0
#include <Common/TString.h>

enum EPropertyType
{
    eBoolProperty,
    eByteProperty,
    eShortProperty,
    eLongProperty,
    eEnumProperty,
    eBitfieldProperty,
    eFloatProperty,
    eStringProperty,
    eVector3Property,
    eColorProperty,
    eSoundProperty,
    eAssetProperty,
    eStructProperty,
    eArrayProperty,
    eCharacterProperty,
    eMayaSplineProperty,
    eUnknownProperty,
    eInvalidProperty
};
#endif

// functions defined in IPropertyTemplate.cpp
EPropertyTypeNew PropStringToPropEnum(TString Prop);
TString PropEnumToPropString(EPropertyTypeNew Prop);
const char* HashablePropTypeName(EPropertyTypeNew Prop);

#endif // EPROPERTYTYPE

