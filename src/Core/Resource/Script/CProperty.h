#ifndef CPROPERTY
#define CPROPERTY

/* This header file declares some classes used to track script object properties
 * CPropertyBase, __CProperty (and typedefs), CPropertyStruct
 * It's a bit hard to read, should be reorganized at some point */
#include "CPropertyTemplate.h"
#include "EPropertyType.h"
#include "Core/Resource/CResource.h"
#include "Core/Resource/TResPtr.h"
#include "Core/Resource/CAnimationParameters.h"
#include <Common/CColor.h>
#include <Common/TString.h>
#include <Math/CVector3f.h>
#include <list>

class CScriptTemplate;

typedef TString TIDString;

/*
 * CPropertyBase is the base class, containing just some virtual function definitions
 * Virtual destructor is mainly there to make cleanup easy; don't need to cast to delete
 */
class CPropertyBase
{
    friend class CScriptLoader;
protected:
    CPropertyTemplate *mpTemplate;
public:
    virtual ~CPropertyBase() {}
    inline virtual EPropertyType Type() = 0;
    inline CPropertyTemplate *Template() { return mpTemplate; }
    inline void SetTemplate(CPropertyTemplate *_tmp) { mpTemplate = _tmp; }
    inline TString Name() { return mpTemplate->Name(); }
    inline u32 ID() { return mpTemplate->PropertyID(); }
};

/*
 * __CProperty is a template subclass for actual properties.
 * Don't use this class directly. Typedefs are provided for every possible property type.
 */
template <typename t, EPropertyType type>
class __CProperty : public CPropertyBase
{
    friend class CScriptLoader;
    t mValue;
public:
    __CProperty() {}
    __CProperty(t v) { Set(v); }
    ~__CProperty() {}
    inline EPropertyType Type() { return type; }
    inline t Get() { return mValue; }
    inline void Set(t v) { mValue = v; }
};
typedef __CProperty<bool, eBoolProperty>                       CBoolProperty;
typedef __CProperty<char, eByteProperty>                       CByteProperty;
typedef __CProperty<short, eShortProperty>                     CShortProperty;
typedef __CProperty<long, eLongProperty>                       CLongProperty;
typedef __CProperty<long, eEnumProperty>                       CEnumProperty;
typedef __CProperty<long, eBitfieldProperty>                   CBitfieldProperty;
typedef __CProperty<float, eFloatProperty>                     CFloatProperty;
typedef __CProperty<TString, eStringProperty>                  CStringProperty;
typedef __CProperty<CVector3f, eVector3Property>               CVector3Property;
typedef __CProperty<CColor, eColorProperty>                    CColorProperty;
typedef __CProperty<TResPtr<CResource>, eFileProperty>         CFileProperty;
typedef __CProperty<CAnimationParameters, eAnimParamsProperty> CAnimParamsProperty;
typedef __CProperty<std::vector<u8>, eArrayProperty>           CArrayProperty;
typedef __CProperty<std::vector<u8>, eUnknownProperty>         CUnknownProperty;

/*
 * CPropertyStruct is for defining structs of properties.
 */
class CPropertyStruct : public CPropertyBase
{
    friend class CScriptLoader;
    std::vector<CPropertyBase*> mProperties;
public:
    // Destructor simply iterates through the list and deletes them. Nothing complicated.
    ~CPropertyStruct();

    // Inline
    EPropertyType Type() { return eStructProperty; }
    inline u32 Count() { return mProperties.size(); }
    inline void Reserve(u32 amount) { mProperties.reserve(amount); }
    inline CPropertyBase* operator[](u32 index) { return mProperties[index]; }

    // Functions
    CPropertyBase* PropertyByIndex(u32 index);
    CPropertyBase* PropertyByID(u32 ID);
    CPropertyBase* PropertyByIDString(const TIDString& str);
    CPropertyStruct* StructByIndex(u32 index);
    CPropertyStruct* StructByID(u32 ID);
    CPropertyStruct* StructByIDString(const TIDString& str);

    // Static
    static CPropertyStruct* CopyFromTemplate(CStructTemplate *pTemp);
};

#endif // CPROPERTY

