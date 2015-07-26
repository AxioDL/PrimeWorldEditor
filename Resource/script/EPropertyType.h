#ifndef EPROPERTYTYPE
#define EPROPERTYTYPE

#include <string>

enum EPropertyType
{
    eBoolProperty,
    eByteProperty,
    eShortProperty,
    eLongProperty,
    eFloatProperty,
    eStringProperty,
    eVector3Property,
    eColorProperty,
    eEnumProperty,
    eFileProperty,
    eStructProperty,
    eUnknownProperty,
    eInvalidProperty
};

// functions defined in CScriptTemplate.cpp
EPropertyType PropStringToPropEnum(std::string prop);
std::string PropEnumToPropString(EPropertyType prop);

#endif // EPROPERTYTYPE

