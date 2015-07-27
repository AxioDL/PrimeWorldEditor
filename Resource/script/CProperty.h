#ifndef CPROPERTY
#define CPROPERTY

/*
 * This header file declares some classes used to track script object properties
 * CPropertyBase, __CProperty (and typedefs), CPropertyStruct
 * It's a bit hard to read, should be reorganized at some point
 */
#include "../CResource.h"
#include "CScriptTemplate.h"
#include "EPropertyType.h"
#include <Common/CColor.h>
#include <Common/CVector3f.h>
#include <Core/CToken.h>
#include <string>
#include <list>

/*
 * CPropertyBase is the base class, containing just some virtual function definitions
 * Virtual destructor is mainly there to make cleanup easy; don't need to cast to delete
 */
class CPropertyBase
{
    friend class CScriptLoader;
protected:
    CPropertyTemplate *tmp;
public:
    virtual ~CPropertyBase() {}
    virtual EPropertyType Type() = 0;
    CPropertyTemplate *Template() { return tmp; }
    void SetTemplate(CPropertyTemplate *_tmp) { tmp = _tmp; }
    std::string Name() { return tmp->Name(); }
    u32 ID() { return tmp->PropertyID(); }
};

/*
 * __CProperty is a template subclass for actual properties.
 * Don't use this class directly. Typedefs are provided for every possible property type.
 */
template <typename t, EPropertyType type>
class __CProperty : public CPropertyBase
{
    friend class CScriptLoader;
    t Value;
public:
    __CProperty() {}
    __CProperty(t v) { Set(v); }
    ~__CProperty() {}
    EPropertyType Type() { return type; }
    t Get() { return Value; }
    void Set(t v) { Value = v; }
};
typedef __CProperty<bool, eBoolProperty>               CBoolProperty;
typedef __CProperty<char, eByteProperty>               CByteProperty;
typedef __CProperty<short, eShortProperty>             CShortProperty;
typedef __CProperty<long, eLongProperty>               CLongProperty;
typedef __CProperty<float, eFloatProperty>             CFloatProperty;
typedef __CProperty<std::string, eStringProperty>      CStringProperty;
typedef __CProperty<CVector3f, eVector3Property>       CVector3Property;
typedef __CProperty<CColor, eColorProperty>            CColorProperty;
typedef __CProperty<CResource*, eFileProperty>         CFileProperty;
typedef __CProperty<std::vector<u8>, eUnknownProperty> CUnknownProperty;

/*
 * Template specialization for CFileProperty to allow a token for resources
 */
template <>
class __CProperty<CResource*, eFileProperty> : public CPropertyBase
{
    CResource *Value;
    CToken mToken;

public:
    __CProperty<CResource*, eFileProperty>() {
        Value = nullptr;
    }

    __CProperty<CResource*, eFileProperty>(CResource* v) {
        Value = v;
        mToken = CToken(v);
    }

    ~__CProperty<CResource*, eFileProperty>() {}

    EPropertyType Type() { return eFileProperty; }
    CResource* Get() { return Value; }
    void Set(CResource *v)
    {
        if (Value != v)
        {
            Value = v;
            mToken = CToken(v);
        }
    }
    const CStringList& AllowedExtensions()
    {
        return static_cast<CFileTemplate*>(Template())->Extensions();
    }
};

/*
 * CPropertyStruct is for defining structs of properties.
 */
class CPropertyStruct : public CPropertyBase
{
    friend class CScriptLoader;
    std::vector<CPropertyBase*> Properties;
public:
    // Destructor simply iterates through the list and deletes them. Nothing complicated.
    ~CPropertyStruct()
    {
        for (auto it = Properties.begin(); it != Properties.end(); it++)
            delete *it;
    }

    EPropertyType Type() { return eStructProperty; }
    u32 Count() { return Properties.size(); }
    void Reserve(u32 amount) { Properties.reserve(amount); }
    CPropertyBase* PropertyByIndex(u32 index) { return Properties[index]; }
    CPropertyBase* PropertyByName(std::string name)
    {
        // Resolve namespace
        std::string::size_type NsStart = name.find_first_of("::");
        std::string::size_type PropStart = NsStart + 2;

        // Namespace; the requested property is within a struct
        if (NsStart != std::string::npos)
        {
            std::string StructName = name.substr(0, NsStart);
            std::string PropName = name.substr(PropStart, name.length() - PropStart);

            CPropertyStruct *Struct = StructByName(StructName);
            if (!Struct) return nullptr;
            else return Struct->PropertyByName(PropName);
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
                for (auto it = Properties.begin(); it != Properties.end(); it++)
                {
                    if ((*it)->Name() == name)
                        return *it;
                }
                return nullptr;
            }
        }
    }
    CPropertyBase* PropertyByID(u32 ID)
    {
        for (auto it = Properties.begin(); it != Properties.end(); it++)
        {
            if ((*it)->ID() == ID)
                return *it;
        }
        return nullptr;
    }
    CPropertyStruct* StructByIndex(u32 index)
    {
        CPropertyBase *prop = PropertyByIndex(index);

        if (prop->Type() == eStructProperty)
            return static_cast<CPropertyStruct*>(prop);
        else
            return nullptr;
    }
    CPropertyStruct* StructByName(std::string name)
    {
        CPropertyBase *prop = PropertyByName(name);

        if (prop->Type() == eStructProperty)
            return static_cast<CPropertyStruct*>(prop);
        else
            return nullptr;
    }
    CPropertyStruct* StructByID(u32 ID)
    {
        CPropertyBase *prop = PropertyByID(ID);

        if (prop->Type() == eStructProperty)
            return static_cast<CPropertyStruct*>(prop);
        else
            return nullptr;
    }
    inline CPropertyBase* operator[](u32 index) { return Properties[index]; }

    static CPropertyStruct* CopyFromTemplate(CStructTemplate *pTemp)
    {
        CPropertyStruct *pStruct = new CPropertyStruct();
        pStruct->tmp = pTemp;
        pStruct->Reserve(pTemp->Count());

        for (u32 iProp = 0; iProp < pTemp->Count(); iProp++)
        {
            CPropertyTemplate *pPropTemp = pTemp->PropertyByIndex(iProp);
            CPropertyBase *pProp = nullptr;

            switch (pPropTemp->Type())
            {
            case eBoolProperty: pProp = new CBoolProperty(false); break;
            case eByteProperty: pProp = new CByteProperty(0);     break;
            case eShortProperty: pProp = new CShortProperty(0); break;
            case eLongProperty: pProp = new CLongProperty(0); break;
            case eFloatProperty: pProp = new CFloatProperty(0.f); break;
            case eStringProperty: pProp = new CStringProperty(""); break;
            case eVector3Property: pProp = new CVector3Property(CVector3f::skZero); break;
            case eColorProperty: pProp = new CColorProperty(CColor::skBlack); break;
            case eFileProperty: pProp = new CFileProperty(); break;
            case eUnknownProperty: pProp = new CUnknownProperty(); break;
            case eStructProperty: pProp = CPropertyStruct::CopyFromTemplate(static_cast<CStructTemplate*>(pPropTemp)); break;
            }

            if (pProp)
            {
                pProp->SetTemplate(pPropTemp);
                pStruct->Properties.push_back(pProp);
            }
        }

        return pStruct;
    }
};

#endif // CPROPERTY

