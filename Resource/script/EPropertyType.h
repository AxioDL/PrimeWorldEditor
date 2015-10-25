#ifndef EPROPERTYTYPE
#define EPROPERTYTYPE

#include <string>

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
    eAnimParamsProperty,
    eUnknownProperty,
    eInvalidProperty
};

// functions defined in CScriptTemplate.cpp
EPropertyType PropStringToPropEnum(std::string prop);
std::string PropEnumToPropString(EPropertyType prop);

#endif // EPROPERTYTYPE

