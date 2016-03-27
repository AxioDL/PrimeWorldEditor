#include "IProperty.h"
#include "IPropertyTemplate.h"

// ************ IProperty ************
bool IProperty::IsInArray() const
{
    CPropertyStruct *pParent = mpParent;

    while (pParent)
    {
        if (pParent->Type() == eArrayProperty) return true;
        pParent = pParent->Parent();
    }

    return false;
}

CPropertyStruct* IProperty::RootStruct()
{
    return (mpParent ? mpParent->RootStruct() : Type() == eStructProperty ? static_cast<CPropertyStruct*>(this) : nullptr);
}

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
    if (mpParent && mpParent->Type() == eArrayProperty)
        return ArrayIndex();
    else
        return mpTemplate->PropertyID();
}

TIDString IProperty::IDString(bool FullPath) const
{
    TIDString Out;

    if (ID() != 0xFFFFFFFF)
    {
        if (mpParent && FullPath)
        {
            Out = mpParent->IDString(true);
            if (!Out.IsEmpty()) Out += ":";
        }

        Out += TString::HexString(ID());
    }

    return Out;
}

u32 IProperty::ArrayIndex() const
{
    CArrayProperty *pArray = TPropCast<CArrayProperty>(mpParent);

    if (pArray)
    {
        for (u32 iSub = 0; iSub < pArray->Count(); iSub++)
        {
            if (pArray->PropertyByIndex(iSub) == this)
                return iSub;
        }
    }

    return -1;
}

bool IProperty::ShouldCook()
{
    if      (mpTemplate->CookPreference() == eNeverCook)    return false;
    else if (mpTemplate->CookPreference() == eAlwaysCook)   return true;

    else
    {
        if (mpTemplate->Game() == eReturns)
            return !MatchesDefault();
        else
            return true;
    }
}

bool IProperty::MatchesDefault()
{
    const IPropertyValue *pkValue = RawValue();
    const IPropertyValue *pkDefault = mpTemplate->RawDefaultValue();
    if (!pkValue || !pkDefault) return false;
    else return pkValue->Matches(pkDefault);
}

// ************ CPropertyStruct ************
void CPropertyStruct::Copy(const IProperty *pkProp)
{
    const CPropertyStruct *pkSource = static_cast<const CPropertyStruct*>(pkProp);

    for (u32 iSub = 0; iSub < mProperties.size(); iSub++)
        mProperties[iSub]->Copy(pkSource->mProperties[iSub]);
}

bool CPropertyStruct::ShouldCook()
{
    if (mpTemplate->CookPreference() == eNeverCook) return false;

    for (u32 iProp = 0; iProp < mProperties.size(); iProp++)
    {
        if (mProperties[iProp]->ShouldCook())
            return true;
    }

    return false;
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

    if (pProp->Type() == eStructProperty || pProp->Type() == eArrayProperty)
        return static_cast<CPropertyStruct*>(pProp);
    else
        return nullptr;
}

CPropertyStruct* CPropertyStruct::StructByID(u32 ID) const
{
    IProperty *pProp = PropertyByID(ID);

    if (pProp->Type() == eStructProperty || pProp->Type() == eArrayProperty)
        return static_cast<CPropertyStruct*>(pProp);
    else
        return nullptr;
}

CPropertyStruct* CPropertyStruct::StructByIDString(const TIDString& rkStr) const
{
    IProperty *pProp = PropertyByIDString(rkStr);

    if (pProp->Type() == eStructProperty || pProp->Type() == eArrayProperty)
        return static_cast<CPropertyStruct*>(pProp);
    else
        return nullptr;
}

// ************ CArrayProperty ************
void CArrayProperty::Copy(const IProperty *pkProp)
{
    const CArrayProperty *pkSource = static_cast<const CArrayProperty*>(pkProp);
    Resize(pkSource->Count());

    for (u32 iSub = 0; iSub < mProperties.size(); iSub++)
        mProperties[iSub]->Copy(pkSource->mProperties[iSub]);
}

bool CArrayProperty::ShouldCook()
{
    return (mpTemplate->CookPreference() == eNeverCook ? false : true);
}

void CArrayProperty::Resize(int Size)
{
    int OldSize = mProperties.size();
    if (OldSize == Size) return;

    if (Size < OldSize)
    {
        for (int iProp = mProperties.size() - 1; iProp >= Size; iProp--)
            delete mProperties[iProp];
    }

    mProperties.resize(Size);

    if (Size > OldSize)
    {
        for (int iProp = OldSize; iProp < Size; iProp++)
            mProperties[iProp] = static_cast<CArrayTemplate*>(mpTemplate)->CreateSubStruct(Instance(), this);
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
