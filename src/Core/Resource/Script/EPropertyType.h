#ifndef EPROPERTYTYPE
#define EPROPERTYTYPE

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
    eFileProperty,
    eStructProperty,
    eArrayProperty,
    eCharacterProperty,
    eMayaSplineProperty,
    eUnknownProperty,
    eInvalidProperty
};

// functions defined in IPropertyTemplate.cpp
EPropertyType PropStringToPropEnum(TString Prop);
TString PropEnumToPropString(EPropertyType Prop);

#endif // EPROPERTYTYPE

