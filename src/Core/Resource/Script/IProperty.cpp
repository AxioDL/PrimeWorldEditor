#include "IProperty.h"
#include "IPropertyTemplate.h"

// ************ IProperty ************
IPropertyTemplate* IProperty::Template() const
{
    return mpTemplate;
}

TString IProperty::Name() const
{
    return mpTemplate->Name();
}

u32 IProperty::ID() const
{
    return mpTemplate->PropertyID();
}

TIDString IProperty::IDString(bool FullPath) const
{
    return mpTemplate->IDString(FullPath);
}

// ************ CPropertyStruct ************
void CPropertyStruct::Copy(const IProperty *pkProp)
{
    const CPropertyStruct *pkSource = static_cast<const CPropertyStruct*>(pkProp);

    for (auto it = mProperties.begin(); it != mProperties.end(); it++)
        delete *it;

    mProperties.resize(pkSource->mProperties.size());

    for (u32 iSub = 0; iSub < mProperties.size(); iSub++)
        mProperties[iSub] = pkSource->mProperties[iSub]->Clone(this);
}

IProperty* CPropertyStruct::PropertyByIndex(u32 index) const
{
    return mProperties[index];
}

IProperty* CPropertyStruct::PropertyByID(u32 ID) const
{
    for (auto it = mProperties.begin(); it != mProperties.end(); it++)
    {
        if ((*it)->ID() == ID)
            return *it;
    }
    return nullptr;
}

IProperty* CPropertyStruct::PropertyByIDString(const TIDString& rkStr) const
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

CPropertyStruct* CPropertyStruct::StructByIndex(u32 index) const
{
    IProperty *pProp = PropertyByIndex(index);

    if (pProp->Type() == eStructProperty)
        return static_cast<CPropertyStruct*>(pProp);
    else
        return nullptr;
}

CPropertyStruct* CPropertyStruct::StructByID(u32 ID) const
{
    IProperty *pProp = PropertyByID(ID);

    if (pProp->Type() == eStructProperty)
        return static_cast<CPropertyStruct*>(pProp);
    else
        return nullptr;
}

CPropertyStruct* CPropertyStruct::StructByIDString(const TIDString& rkStr) const
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
    u32 OldSize = mProperties.size();
    if (OldSize == Size) return;

    if (Size < OldSize)
    {
        for (u32 i = mProperties.size() - 1; i >= Size; i--)
            delete mProperties[i];
    }

    mProperties.resize(Size);

    if (Size > OldSize)
    {
        for (u32 i = OldSize; i < Size; i++)
            mProperties[i] = static_cast<CArrayTemplate*>(mpTemplate)->CreateSubStruct(this);
    }
}

CStructTemplate* CArrayProperty::SubStructTemplate() const
{
    // CArrayTemplate inherits from CStructTemplate. The template defines the substruct structure.
    return static_cast<CStructTemplate*>(Template());
}

TString CArrayProperty::ElementName() const
{
    return static_cast<CArrayTemplate*>(Template())->ElementName();
}
