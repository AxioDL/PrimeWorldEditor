#ifndef EATTRIBTYPE
#define EATTRIBTYPE

#include <Common/EnumUtil.h>
#include <string>

enum EAttribType
{
    eNameAttrib          = 0x1,
    ePositionAttrib      = 0x2,
    eRotationAttrib      = 0x4,
    eScaleAttrib         = 0x8,
    eModelAttrib         = 0x10,
    eAnimSetAttrib       = 0x20,
    eVolumeAttrib        = 0x40,
    eVulnerabilityAttrib = 0x80,
    eInvalidAttrib       = 0x80000000
};
DEFINE_ENUM_FLAGS(EAttribType)

// functions defined in CScriptTemplate.cpp
EAttribType AttribStringToAttribEnum(const std::string& Attrib);
std::string AttribEnumToAttribString(EAttribType Attrib);

#endif // EATTRIBTYPE

