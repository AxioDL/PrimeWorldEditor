#include "CPropertyTemplate.h"
#include <iostream>

EPropertyType PropStringToPropEnum(std::string prop)
{
    if (prop == "bool")       return eBoolProperty;
    if (prop == "byte")       return eByteProperty;
    if (prop == "short")      return eShortProperty;
    if (prop == "long")       return eLongProperty;
    if (prop == "enum")       return eEnumProperty;
    if (prop == "float")      return eFloatProperty;
    if (prop == "string")     return eStringProperty;
    if (prop == "color")      return eColorProperty;
    if (prop == "vector3f")   return eVector3Property;
    if (prop == "file")       return eFileProperty;
    if (prop == "struct")     return eStructProperty;
    if (prop == "array")      return eArrayProperty;
    if (prop == "animparams") return eAnimParamsProperty;
    if (prop == "unknown")    return eUnknownProperty;
                              return eInvalidProperty;
}

std::string PropEnumToPropString(EPropertyType prop)
{
    switch (prop)
    {
    case eBoolProperty:       return "bool";
    case eByteProperty:       return "byte";
    case eShortProperty:      return "short";
    case eLongProperty:       return "long";
    case eEnumProperty:       return "enum";
    case eFloatProperty:      return "float";
    case eStringProperty:     return "string";
    case eColorProperty:      return "color";
    case eVector3Property:    return "vector3f";
    case eFileProperty:       return "file";
    case eStructProperty:     return "struct";
    case eArrayProperty:      return "array";
    case eAnimParamsProperty: return "animparams";
    case eUnknownProperty:    return "unknown";

    case eInvalidProperty:
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

CPropertyTemplate* CStructTemplate::PropertyByID(u32 ID)
{
    for (auto it = mProperties.begin(); it != mProperties.end(); it++)
    {
        if ((*it)->PropertyID() == ID)
            return *it;
    }
    return nullptr;
}

CPropertyTemplate* CStructTemplate::PropertyByIDString(const std::string& str)
{
    // Resolve namespace
    std::string::size_type nsStart = str.find_first_of("::");
    std::string::size_type propStart = nsStart + 2;

    // String has namespace; the requested property is within a struct
    if (nsStart != std::string::npos)
    {
        std::string strStructID = str.substr(0, nsStart);
        if (!StringUtil::IsHexString(strStructID)) return nullptr;

        u32 structID = StringUtil::ToInt32(strStructID);
        std::string propName = str.substr(propStart, str.length() - propStart);

        CStructTemplate *pStruct = StructByID(structID);
        if (!pStruct) return nullptr;
        else return pStruct->PropertyByIDString(propName);
    }

    // No namespace; fetch the property from this struct
    else
    {
        // ID string lookup
        if (StringUtil::IsHexString(str))
            return PropertyByID(std::stoul(str, 0, 16));
        else
            return nullptr;
    }
}

CStructTemplate* CStructTemplate::StructByIndex(u32 index)
{
    CPropertyTemplate *pProp = PropertyByIndex(index);

    if (pProp->Type() == eStructProperty)
        return static_cast<CStructTemplate*>(pProp);
    else
        return nullptr;
}

CStructTemplate* CStructTemplate::StructByID(u32 ID)
{
    CPropertyTemplate *pProp = PropertyByID(ID);

    if (pProp && pProp->Type() == eStructProperty)
        return static_cast<CStructTemplate*>(pProp);
    else
        return nullptr;
}

CStructTemplate* CStructTemplate::StructByIDString(const std::string& str)
{
    CPropertyTemplate *pProp = PropertyByIDString(str);

    if (pProp && pProp->Type() == eStructProperty)
        return static_cast<CStructTemplate*>(pProp);
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
