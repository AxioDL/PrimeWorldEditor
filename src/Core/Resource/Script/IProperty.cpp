#include "IProperty.h"
#include "IPropertyTemplate.h"

// ************ IProperty ************
IPropertyTemplate* IProperty::Template()
{
    return mpTemplate;
}

TString IProperty::Name()
{
    return mpTemplate->Name();
}

u32 IProperty::ID()
{
    return mpTemplate->PropertyID();
}

// ************ CPropertyStruct ************
CPropertyStruct::~CPropertyStruct()
{
    for (auto it = mProperties.begin(); it != mProperties.end(); it++)
        delete *it;
}

IProperty* CPropertyStruct::PropertyByIndex(u32 index)
{
    return mProperties[index];
}

IProperty* CPropertyStruct::PropertyByID(u32 ID)
{
    for (auto it = mProperties.begin(); it != mProperties.end(); it++)
    {
        if ((*it)->ID() == ID)
            return *it;
    }
    return nullptr;
}

IProperty* CPropertyStruct::PropertyByIDString(const TIDString& rkStr)
{
    // Resolve namespace
    u32 NSStart = rkStr.IndexOf(":");

    // String has namespace; the requested property is within a struct
    if (NSStart != -1)
    {
        TString StrStructID = rkStr.Truncate(NSStart);
        if (!StrStructID.IsHexString()) return nullptr;

        u32 StructID = StrStructID.ToInt32();
        TString PropName = rkStr.ChopFront(NSStart + 1);

        CPropertyStruct *pStruct = StructByID(StructID);
        if (!pStruct) return nullptr;
        else return pStruct->PropertyByIDString(PropName);
    }

    // No namespace; fetch the property from this struct
    else
    {
        if (rkStr.IsHexString())
            return PropertyByID(rkStr.ToInt32());
        else
            return nullptr;
    }
}

CPropertyStruct* CPropertyStruct::StructByIndex(u32 index)
{
    IProperty *pProp = PropertyByIndex(index);

    if (pProp->Type() == eStructProperty)
        return static_cast<CPropertyStruct*>(pProp);
    else
        return nullptr;
}

CPropertyStruct* CPropertyStruct::StructByID(u32 ID)
{
    IProperty *pProp = PropertyByID(ID);

    if (pProp->Type() == eStructProperty)
        return static_cast<CPropertyStruct*>(pProp);
    else
        return nullptr;
}

CPropertyStruct* CPropertyStruct::StructByIDString(const TIDString& rkStr)
{
    IProperty *pProp = PropertyByIDString(rkStr);

    if (pProp->Type() == eStructProperty)
        return static_cast<CPropertyStruct*>(pProp);
    else
        return nullptr;
}

// ************ CArrayProperty ************
void CArrayProperty::Resize(u32 Size)
{
    u32 OldSize = mSubStructs.size();
    if (OldSize == Size) return;

    if (Size < OldSize)
    {
        for (u32 i = mSubStructs.size() - 1; i >= Size; i--)
            delete mSubStructs[i];
    }

    mSubStructs.resize(Size);

    if (Size > OldSize)
    {
        for (u32 i = OldSize; i < Size; i++)
            mSubStructs[i] = static_cast<CArrayTemplate*>(mpTemplate)->CreateSubStruct();
    }
}

CStructTemplate* CArrayProperty::SubStructTemplate()
{
    // CArrayTemplate inherits from CStructTemplate. It defines the substruct structure.
    return static_cast<CStructTemplate*>(Template());
}
