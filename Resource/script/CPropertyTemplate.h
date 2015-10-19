#ifndef CPROPERTYTEMPLATE
#define CPROPERTYTEMPLATE

#include "EPropertyType.h"
#include <Common/StringUtil.h>
#include <Common/types.h>
#include <string>
#include <vector>

class CPropertyTemplate
{
    friend class CTemplateLoader;
    friend class CTemplateWriter;

protected:
    EPropertyType mPropType;
    std::string mPropName;
    u32 mPropID;
public:
    CPropertyTemplate(u32 ID)
        : mPropID(ID)
    {
    }

    CPropertyTemplate(EPropertyType type, std::string name, u32 ID)
        : mPropType(type),
          mPropName(name),
          mPropID(ID)
    {
    }

    virtual EPropertyType Type() const
    {
        return mPropType;
    }

    inline std::string Name() const
    {
        return mPropName;
    }

    inline u32 PropertyID() const
    {
        return mPropID;
    }

    inline void SetName(const std::string& Name)
    {
        mPropName = Name;
    }
};

class CFileTemplate : public CPropertyTemplate
{
    friend class CTemplateLoader;
    friend class CTemplateWriter;

    CStringList mAcceptedExtensions;
public:
    CFileTemplate(u32 ID) : CPropertyTemplate(ID) { mPropType = eFileProperty; }

    CFileTemplate(const std::string& name, u32 ID, const CStringList& extensions)
        : CPropertyTemplate(ID)
    {
        mPropType = eFileProperty;
        mPropName = name;
        mAcceptedExtensions = extensions;
    }

    EPropertyType Type() const
    {
        return eFileProperty;
    }

    const CStringList& Extensions() const
    {
        return mAcceptedExtensions;
    }
};

class CEnumTemplate : public CPropertyTemplate
{
    friend class CTemplateLoader;
    friend class CTemplateWriter;

    struct SEnumerator
    {
        std::string Name;
        u32 ID;

        SEnumerator(const std::string& _name, u32 _ID)
            : Name(_name), ID(_ID) {}
    };
    std::vector<SEnumerator> mEnumerators;
    std::string mSourceFile;

public:
    CEnumTemplate(u32 ID)
        : CPropertyTemplate(ID)
    {
        mPropType = eEnumProperty;
    }

    CEnumTemplate(const std::string& name, u32 ID)
        : CPropertyTemplate(eEnumProperty, name, ID)
    {}

    EPropertyType Type() const
    {
        return eEnumProperty;
    }

    u32 NumEnumerators()
    {
        return mEnumerators.size();
    }

    u32 EnumeratorIndex(u32 enumID)
    {
        for (u32 iEnum = 0; iEnum < mEnumerators.size(); iEnum++)
        {
            if (mEnumerators[iEnum].ID == enumID)
                return iEnum;
        }
        return -1;
    }

    u32 EnumeratorID(u32 enumIndex)
    {
        if (mEnumerators.size() > enumIndex)
            return mEnumerators[enumIndex].ID;

        else return -1;
    }

    std::string EnumeratorName(u32 enumIndex)
    {
        if (mEnumerators.size() > enumIndex)
            return mEnumerators[enumIndex].Name;

        else return "INVALID ENUM INDEX";
    }
};

class CStructTemplate : public CPropertyTemplate
{
    friend class CTemplateLoader;
    friend class CTemplateWriter;

    bool mIsSingleProperty;
    std::vector<CPropertyTemplate*> mProperties;
    std::string mSourceFile;
public:
    CStructTemplate();
    ~CStructTemplate();

    EPropertyType Type() const;
    bool IsSingleProperty() const;
    u32 Count() const;
    CPropertyTemplate* PropertyByIndex(u32 index);
    CPropertyTemplate* PropertyByID(u32 ID);
    CPropertyTemplate* PropertyByIDString(const std::string& str);
    CStructTemplate* StructByIndex(u32 index);
    CStructTemplate* StructByID(u32 ID);
    CStructTemplate* StructByIDString(const std::string& str);
   void DebugPrintProperties(std::string base);
};

#endif // CPROPERTYTEMPLATE

