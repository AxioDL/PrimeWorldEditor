#include "CProperty.h"

// ************ CPropertyStruct ************
CPropertyStruct::~CPropertyStruct()
{
    for (auto it = mProperties.begin(); it != mProperties.end(); it++)
        delete *it;
}

CPropertyBase* CPropertyStruct::PropertyByIndex(u32 index)
{
    return mProperties[index];
}

CPropertyBase* CPropertyStruct::PropertyByID(u32 ID)
{
    for (auto it = mProperties.begin(); it != mProperties.end(); it++)
    {
        if ((*it)->ID() == ID)
            return *it;
    }
    return nullptr;
}

CPropertyBase* CPropertyStruct::PropertyByIDString(const TIDString& str)
{
    // Resolve namespace
    std::string::size_type nsStart = str.find_first_of(":");
    std::string::size_type propStart = nsStart + 1;

    // String has namespace; the requested property is within a struct
    if (nsStart != std::string::npos)
    {
        std::string strStructID = str.substr(0, nsStart);
        if (!StringUtil::IsHexString(strStructID)) return nullptr;

        u32 structID = StringUtil::ToInt32(strStructID);
        std::string propName = str.substr(propStart, str.length() - propStart);

        CPropertyStruct *pStruct = StructByID(structID);
        if (!pStruct) return nullptr;
        else return pStruct->PropertyByIDString(propName);
    }

    // No namespace; fetch the property from this struct
    else
    {
        if (StringUtil::IsHexString(str))
            return PropertyByID(StringUtil::ToInt32(str));
        else
            return nullptr;
    }
}

CPropertyStruct* CPropertyStruct::StructByIndex(u32 index)
{
    CPropertyBase *pProp = PropertyByIndex(index);

    if (pProp->Type() == eStructProperty)
        return static_cast<CPropertyStruct*>(pProp);
    else
        return nullptr;
}

CPropertyStruct* CPropertyStruct::StructByID(u32 ID)
{
    CPropertyBase *pProp = PropertyByID(ID);

    if (pProp->Type() == eStructProperty)
        return static_cast<CPropertyStruct*>(pProp);
    else
        return nullptr;
}

CPropertyStruct* CPropertyStruct::StructByIDString(const TIDString& str)
{
    CPropertyBase *pProp = PropertyByIDString(str);

    if (pProp->Type() == eStructProperty)
        return static_cast<CPropertyStruct*>(pProp);
    else
        return nullptr;
}

// ************ STATIC ************
CPropertyStruct* CPropertyStruct::CopyFromTemplate(CStructTemplate *pTemp)
{
    CPropertyStruct *pStruct = new CPropertyStruct();
    pStruct->mpTemplate = pTemp;
    pStruct->Reserve(pTemp->Count());

    for (u32 iProp = 0; iProp < pTemp->Count(); iProp++)
    {
        CPropertyTemplate *pPropTemp = pTemp->PropertyByIndex(iProp);
        CPropertyBase *pProp = nullptr;

        switch (pPropTemp->Type())
        {
        case eBoolProperty:       pProp = new CBoolProperty(false); break;
        case eByteProperty:       pProp = new CByteProperty(0);     break;
        case eShortProperty:      pProp = new CShortProperty(0); break;
        case eLongProperty:       pProp = new CLongProperty(0); break;
        case eFloatProperty:      pProp = new CFloatProperty(0.f); break;
        case eStringProperty:     pProp = new CStringProperty(""); break;
        case eVector3Property:    pProp = new CVector3Property(CVector3f::skZero); break;
        case eColorProperty:      pProp = new CColorProperty(CColor::skBlack); break;
        case eFileProperty:       pProp = new CFileProperty(); break;
        case eArrayProperty:      pProp = new CArrayProperty(); break;
        case eAnimParamsProperty: pProp = new CAnimParamsProperty(); break;
        case eUnknownProperty:    pProp = new CUnknownProperty(); break;
        case eStructProperty:     pProp = CPropertyStruct::CopyFromTemplate(static_cast<CStructTemplate*>(pPropTemp)); break;
        }

        if (pProp)
        {
            pProp->SetTemplate(pPropTemp);
            pStruct->mProperties.push_back(pProp);
        }
    }

    return pStruct;
}
