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
    eUnknownProperty,
    eInvalidProperty
};

// functions defined in CScriptTemplate.cpp
EPropertyType PropStringToPropEnum(const TString& prop);
TString PropEnumToPropString(EPropertyType prop);

#endif // EPROPERTYTYPE

