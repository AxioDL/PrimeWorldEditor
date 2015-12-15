#include "CPropertyTemplate.h"
#include <iostream>

EPropertyType PropStringToPropEnum(const TString& prop)
{
    if (prop == "bool")       return eBoolProperty;
    if (prop == "byte")       return eByteProperty;
    if (prop == "short")      return eShortProperty;
    if (prop == "long")       return eLongProperty;
    if (prop == "enum")       return eEnumProperty;
    if (prop == "bitfield")   return eBitfieldProperty;
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

TString PropEnumToPropString(EPropertyType prop)
{
    switch (prop)
    {
    case eBoolProperty:       return "bool";
    case eByteProperty:       return "byte";
    case eShortProperty:      return "short";
    case eLongProperty:       return "long";
    case eEnumProperty:       return "enum";
    case eBitfieldProperty:   return "bitfield";
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

CPropertyTemplate* CStructTemplate::PropertyByIDString(const TIDString& str)
{
    // Resolve namespace
    u32 nsStart = str.IndexOf(":");
    u32 propStart = nsStart + 1;

    // String has namespace; the requested property is within a struct
    if (nsStart != -1)
    {
        TString strStructID = str.SubString(0, nsStart);
        if (!strStructID.IsHexString()) return nullptr;

        u32 structID = strStructID.ToInt32();
        TString propName = str.SubString(propStart, str.Length() - propStart);

        CStructTemplate *pStruct = StructByID(structID);
        if (!pStruct) return nullptr;
        else return pStruct->PropertyByIDString(propName);
    }

    // No namespace; fetch the property from this struct
    else
    {
        // ID string lookup
        if (str.IsHexString())
            return PropertyByID(str.ToInt32());
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

CStructTemplate* CStructTemplate::StructByIDString(const TString& str)
{
    CPropertyTemplate *pProp = PropertyByIDString(str);

    if (pProp && pProp->Type() == eStructProperty)
        return static_cast<CStructTemplate*>(pProp);
    else
        return nullptr;
}

// ************ DEBUG ************
void CStructTemplate::DebugPrintProperties(TString base)
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
