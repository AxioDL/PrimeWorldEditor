#include "IPropertyTemplate.h"
#include <iostream>

// ************ IPropertyTemplate ************
bool IPropertyTemplate::IsInVersion(u32 Version) const
{
    if (mAllowedVersions.empty())
        return true;

    for (u32 iVer = 0; iVer < mAllowedVersions.size(); iVer++)
        if (mAllowedVersions[iVer] == Version)
            return true;

    return false;
}

TIDString IPropertyTemplate::IDString(bool FullPath) const
{
    if (mID != 0xFFFFFFFF)
    {
        TIDString out;

        if (mpParent && FullPath)
        {
            out = mpParent->IDString(true);
            if (!out.IsEmpty()) out += ":";
        }

        out += TIDString::HexString(mID, true, true, 8);
        return out;
    }
    else return "";
}

CStructTemplate* IPropertyTemplate::RootStruct()
{
    if (mpParent) return mpParent->RootStruct();
    else if (Type() == eStructProperty) return static_cast<CStructTemplate*>(this);
    else return nullptr;
}

// ************ CStructTemplate ************
bool CStructTemplate::IsSingleProperty() const
{
    return mIsSingleProperty;
}

u32 CStructTemplate::Count() const
{
    return mSubProperties.size();
}

u32 CStructTemplate::NumVersions()
{
    return mVersionPropertyCounts.size();
}

u32 CStructTemplate::PropertyCountForVersion(u32 Version)
{
    if (Version == -1) Version = 0;
    return mVersionPropertyCounts[Version];
}

u32 CStructTemplate::VersionForPropertyCount(u32 PropCount)
{
    for (u32 iVer = 0; iVer < NumVersions(); iVer++)
        if (mVersionPropertyCounts[iVer] == PropCount)
            return iVer;

    return -1;
}

IPropertyTemplate* CStructTemplate::PropertyByIndex(u32 index)
{
    if (mSubProperties.size() > index)
        return mSubProperties[index];
    else
        return nullptr;
}

IPropertyTemplate* CStructTemplate::PropertyByID(u32 ID)
{
    for (auto it = mSubProperties.begin(); it != mSubProperties.end(); it++)
    {
        if ((*it)->PropertyID() == ID)
            return *it;
    }
    return nullptr;
}

IPropertyTemplate* CStructTemplate::PropertyByIDString(const TIDString& str)
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
    IPropertyTemplate *pProp = PropertyByIndex(index);

    if (pProp->Type() == eStructProperty)
        return static_cast<CStructTemplate*>(pProp);
    else
        return nullptr;
}

CStructTemplate* CStructTemplate::StructByID(u32 ID)
{
    IPropertyTemplate *pProp = PropertyByID(ID);

    if (pProp && pProp->Type() == eStructProperty)
        return static_cast<CStructTemplate*>(pProp);
    else
        return nullptr;
}

CStructTemplate* CStructTemplate::StructByIDString(const TString& str)
{
    IPropertyTemplate *pProp = PropertyByIDString(str);

    if (pProp && pProp->Type() == eStructProperty)
        return static_cast<CStructTemplate*>(pProp);
    else
        return nullptr;
}

bool CStructTemplate::HasProperty(const TIDString& rkIdString)
{
    IPropertyTemplate *pProperty = PropertyByIDString(rkIdString);
    return (pProperty != nullptr);
}

void CStructTemplate::DetermineVersionPropertyCounts()
{
    for (u32 iVer = 0; iVer < mVersionPropertyCounts.size(); iVer++)
    {
        mVersionPropertyCounts[iVer] = 0;

        for (u32 iProp = 0; iProp < mSubProperties.size(); iProp++)
        {
            if (mSubProperties[iProp]->IsInVersion(iVer) && mSubProperties[iProp]->CookPreference() != eNeverCook)
                mVersionPropertyCounts[iVer]++;
        }
    }
}

// ************ GLOBAL FUNCTIONS ************
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
    case eCharacterProperty:  return "character";
    case eUnknownProperty:    return "unknown";

    case eInvalidProperty:
    default:
        return "invalid";
    }
}

EPropertyType PropStringToPropEnum(const TString& rkProp)
{
    if (rkProp == "bool")       return eBoolProperty;
    if (rkProp == "byte")       return eByteProperty;
    if (rkProp == "short")      return eShortProperty;
    if (rkProp == "long")       return eLongProperty;
    if (rkProp == "enum")       return eEnumProperty;
    if (rkProp == "bitfield")   return eBitfieldProperty;
    if (rkProp == "float")      return eFloatProperty;
    if (rkProp == "string")     return eStringProperty;
    if (rkProp == "color")      return eColorProperty;
    if (rkProp == "vector3f")   return eVector3Property;
    if (rkProp == "file")       return eFileProperty;
    if (rkProp == "struct")     return eStructProperty;
    if (rkProp == "array")      return eArrayProperty;
    if (rkProp == "character")  return eCharacterProperty;
    if (rkProp == "unknown")    return eUnknownProperty;
                              return eInvalidProperty;
}

// ************ DEBUG ************
void CStructTemplate::DebugPrintProperties(TString base)
{
    base = base + Name() + "::";
    for (auto it = mSubProperties.begin(); it != mSubProperties.end(); it++)
    {
        IPropertyTemplate *tmp = *it;
        if (tmp->Type() == eStructProperty)
        {
            CStructTemplate *tmp2 = static_cast<CStructTemplate*>(tmp);
            tmp2->DebugPrintProperties(base);
        }
        else
            std::cout << base << tmp->Name() << "\n";
    }
}
