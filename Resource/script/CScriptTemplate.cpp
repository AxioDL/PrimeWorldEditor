#include "CScriptTemplate.h"
#include "CScriptObject.h"
#include "CMasterTemplate.h"
#include <iostream>
#include <string>
#include <Core/Log.h>
#include <Core/CResCache.h>

EPropertyType PropStringToPropEnum(std::string prop)
{
    if (prop == "bool")     return eBoolProperty;
    if (prop == "byte")     return eByteProperty;
    if (prop == "short")    return eShortProperty;
    if (prop == "long")     return eLongProperty;
    if (prop == "float")    return eFloatProperty;
    if (prop == "string")   return eStringProperty;
    if (prop == "color")    return eColorProperty;
    if (prop == "vector3f") return eVector3Property;
    if (prop == "file")     return eFileProperty;
    if (prop == "struct")   return eStructProperty;
    if (prop == "unknown")  return eUnknownProperty;
                            return eInvalidProperty;
}

std::string PropEnumToPropString(EPropertyType prop)
{
    switch (prop)
    {
    case eBoolProperty:    return "bool";
    case eByteProperty:    return "byte";
    case eShortProperty:   return "short";
    case eLongProperty:    return "long";
    case eFloatProperty:   return "float";
    case eStringProperty:  return "string";
    case eColorProperty:   return "color";
    case eVector3Property: return "vector3f";
    case eFileProperty:    return "file";
    case eStructProperty:  return "struct";
    case eUnknownProperty: return "unknown";

    case eInvalidProperty:
    default:
        return "invalid";
    }
}

EAttribType AttribStringToAttribEnum(const std::string& Attrib)
{
    if (Attrib == "name")          return eNameAttrib;
    if (Attrib == "position")      return ePositionAttrib;
    if (Attrib == "rotation")      return eRotationAttrib;
    if (Attrib == "scale")         return eScaleAttrib;
    if (Attrib == "model")         return eModelAttrib;
    if (Attrib == "animset")       return eAnimSetAttrib;
    if (Attrib == "volume")        return eVolumeAttrib;
    if (Attrib == "vulnerability") return eVulnerabilityAttrib;
                                   return eInvalidAttrib;
}

std::string AttribEnumToAttribString(EAttribType Attrib)
{
    switch (Attrib)
    {
    case eNameAttrib:          return "name";
    case ePositionAttrib:      return "position";
    case eRotationAttrib:      return "rotation";
    case eScaleAttrib:         return "scale";
    case eModelAttrib:         return "model";
    case eAnimSetAttrib:       return "animset";
    case eVolumeAttrib:        return "volume";
    case eVulnerabilityAttrib: return "vulnerability";

    case eInvalidAttrib:
    default:
        return "invalid";
    }
}

/*******************
 * CStructTemplate *
 *******************/
CStructTemplate::CStructTemplate() : CPropertyTemplate(-1)
{
    mIsSingleProperty = false;
    mPropertyCount = -1;
    mPropType = eStructProperty;
}

CStructTemplate::~CStructTemplate()
{
    for (auto it = mProperties.begin(); it != mProperties.end(); it++)
        delete *it;
}

// ************ GETTERS ************
EPropertyType CStructTemplate::Type() const
{
    return eStructProperty;
}

bool CStructTemplate::IsSingleProperty() const
{
    return mIsSingleProperty;
}

s32 CStructTemplate::TemplateCount() const
{
    return mPropertyCount;
}

u32 CStructTemplate::Count() const
{
    return mProperties.size();
}

CPropertyTemplate* CStructTemplate::PropertyByIndex(u32 index)
{
    if (mProperties.size() > index)
        return mProperties[index];
    else
        return nullptr;
}

CPropertyTemplate* CStructTemplate::PropertyByName(std::string name)
{
    // Resolve namespace
    std::string::size_type NsStart = name.find_first_of("::");
    std::string::size_type PropStart = NsStart + 2;

    // Namespace; the requested property is within a struct
    if (NsStart != std::string::npos)
    {
        std::string StructName = name.substr(0, NsStart);
        std::string PropName = name.substr(PropStart, name.length() - PropStart);

        CStructTemplate *tmp = StructByName(StructName);
        if (!tmp) return nullptr;
        else return tmp->PropertyByName(PropName);
    }

    // No namespace; fetch the property from this struct
    else
    {
        // ID string lookup
        if (StringUtil::IsHexString(name))
            return PropertyByID(std::stoul(name, 0, 16));

        // Name lookup
        else
        {
            for (auto it = mProperties.begin(); it != mProperties.end(); it++)
            {
                if ((*it)->Name() == name)
                    return *it;
            }
            return nullptr;
        }
    }
}

CPropertyTemplate* CStructTemplate::PropertyByID(u32 ID)
{
    for (auto it = mProperties.begin(); it != mProperties.end(); it++)
    {
        if ((*it)->PropertyID() == ID)
            return *it;
    }
    return nullptr;
}

CStructTemplate* CStructTemplate::StructByIndex(u32 index)
{
    CPropertyTemplate *prop = PropertyByIndex(index);

    if (prop->Type() == eStructProperty)
        return static_cast<CStructTemplate*>(prop);
    else
        return nullptr;
}

CStructTemplate* CStructTemplate::StructByName(std::string name)
{
    CPropertyTemplate *prop = PropertyByName(name);

    if (prop && prop->Type() == eStructProperty)
        return static_cast<CStructTemplate*>(prop);
    else
        return nullptr;
}

CStructTemplate* CStructTemplate::StructByID(u32 ID)
{
    CPropertyTemplate *prop = PropertyByID(ID);

    if (prop && prop->Type() == eStructProperty)
        return static_cast<CStructTemplate*>(prop);
    else
        return nullptr;
}

// ************ DEBUG ************
void CStructTemplate::DebugPrintProperties(std::string base)
{
    base = base + Name() + "::";
    for (auto it = mProperties.begin(); it != mProperties.end(); it++)
    {
        CPropertyTemplate *tmp = *it;
        if (tmp->Type() == eStructProperty)
        {
            CStructTemplate *tmp2 = static_cast<CStructTemplate*>(tmp);
            tmp2->DebugPrintProperties(base);
        }
        else
            std::cout << base << tmp->Name() << "\n";
    }
}

/*******************
 * CScriptTemplate *
 *******************/
CScriptTemplate::CScriptTemplate(CMasterTemplate *pMaster)
{
    mpBaseStruct = nullptr;
    mpMaster = pMaster;
    mVisible = true;
}

CScriptTemplate::~CScriptTemplate()
{
    if (mpBaseStruct)
        delete mpBaseStruct;
}

CMasterTemplate* CScriptTemplate::MasterTemplate()
{
    return mpMaster;
}

std::string CScriptTemplate::TemplateName() const
{
    return mTemplateName;
}

CStructTemplate* CScriptTemplate::BaseStruct()
{
    return mpBaseStruct;
}

u32 CScriptTemplate::AttribCount() const
{
    return mAttribs.size();
}

CAttribTemplate* CScriptTemplate::Attrib(u32 index)
{
    if (mAttribs.size() > index)
        return &mAttribs[index];
    else
        return nullptr;
}

u32 CScriptTemplate::ObjectID() const
{
    return mObjectID;
}

u32 CScriptTemplate::NumObjects() const
{
    return mObjectList.size();
}

const std::list<CScriptObject*>& CScriptTemplate::ObjectList() const
{
    return mObjectList;
}

void CScriptTemplate::AddObject(CScriptObject *pObject)
{
    mObjectList.push_back(pObject);
}

void CScriptTemplate::RemoveObject(CScriptObject *pObject)
{
    for (auto it = mObjectList.begin(); it != mObjectList.end(); it++)
    {
        if (*it == pObject)
        {
            mObjectList.erase(it);
            break;
        }
    }
}

void CScriptTemplate::SortObjects()
{
    // todo: make this function take layer names into account
    mObjectList.sort([](CScriptObject *pA, CScriptObject *pB) -> bool {
        return (pA->InstanceID() < pB->InstanceID());
    });
}

void CScriptTemplate::SetVisible(bool Visible)
{
    mVisible = Visible;
}

bool CScriptTemplate::IsVisible()
{
    return mVisible;
}

// Debug function
void CScriptTemplate::DebugPrintProperties()
{
    mpBaseStruct->DebugPrintProperties("");
}
