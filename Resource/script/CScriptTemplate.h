#ifndef CSCRIPTTEMPLATE_H
#define CSCRIPTTEMPLATE_H

#include "EPropertyType.h"
#include "EAttribType.h"
#include <Common/CFourCC.h>
#include <Common/types.h>
#include <list>
#include <vector>
#include <tinyxml2.h>
#include <Resource/CResource.h>

class CMasterTemplate;

/**
 * CPropertyTemplate and CStructTemplate each define the layout of a single property/struct.
 * The reason they're classes instead of structs is so their internal values can't be externally modified.
 * CFileTemplate is a very simple subclass with one extra value - a file extension fourCC
 */
class CPropertyTemplate
{
    friend class CTemplateLoader;

protected:
    EPropertyType mPropType;
    std::string mPropName;
    u32 mPropID;
public:
    CPropertyTemplate(u32 ID) { mPropID = ID; }
    CPropertyTemplate(EPropertyType type, std::string name, u32 ID) : mPropType(type), mPropName(name), mPropID(ID) {}

    virtual EPropertyType Type() const { return mPropType; }
    inline std::string Name() const { return mPropName; }
    inline u32 PropertyID() const { return mPropID; }
    inline void SetName(const std::string& Name) { mPropName = Name; }
};

class CFileTemplate : public CPropertyTemplate
{
    friend class CTemplateLoader;

    CStringList mAcceptedExtensions;
public:
    CFileTemplate(u32 ID) : CPropertyTemplate(ID) { mPropType = eFileProperty; }

    CFileTemplate(std::string name, u32 ID, const CStringList& extensions)
        : CPropertyTemplate(ID) {
        mPropType = eFileProperty; mPropName = name; mAcceptedExtensions = extensions;
    }

    EPropertyType Type() const { return eFileProperty; }
    const CStringList& Extensions() const { return mAcceptedExtensions; }
};

class CStructTemplate : public CPropertyTemplate
{
    friend class CTemplateLoader;

    bool mIsSingleProperty;
    s32 mPropertyCount;
    std::vector<CPropertyTemplate*> mProperties;
public:
    CStructTemplate();
    ~CStructTemplate();

    EPropertyType Type() const;
    bool IsSingleProperty() const;
    s32 TemplateCount() const;
    u32 Count() const;
    CPropertyTemplate* PropertyByIndex(u32 index);
    CPropertyTemplate* PropertyByName(std::string name);
    CPropertyTemplate* PropertyByID(u32 ID);
    CStructTemplate* StructByIndex(u32 index);
    CStructTemplate* StructByName(std::string name);
    CStructTemplate* StructByID(u32 ID);
   void DebugPrintProperties(std::string base);
};

/**
 * CAttribTemplate defines editor attributes.
 * They enable PWE to access and use object properties for use in the world editor.
 */
class CAttribTemplate
{
    friend class CTemplateLoader;

    EAttribType AttribType;
    std::string AttribTarget;
    std::string ResFile;
    u32 ExtraSettings;
public:
    CAttribTemplate() {}
    EAttribType Type() const { return AttribType; }
    std::string Target() const { return AttribTarget; }
    std::string Resource() const { return ResFile; }
    u32 Settings() const { return ExtraSettings; }
};

/**
 * CScriptTemplate is a class that encases the data contained in one of the XML templates.
 * It essentially sets the layout of any given script object.
 *
 * It contains any data that applies globally to every instance of the object, such as
 *   property names, editor attribute properties, etc.
 */
class CScriptObject;

class CScriptTemplate
{
    friend class CTemplateLoader;

    CMasterTemplate *mpMaster;
    CStructTemplate *mpBaseStruct;
    std::list<CScriptObject*> mObjectList;
    std::string mTemplateName;
    std::vector<CAttribTemplate> mAttribs;
    u32 mObjectID;
    bool mVisible;

public:
    CScriptTemplate(CMasterTemplate *pMaster);
    ~CScriptTemplate();

    CMasterTemplate* MasterTemplate();
    std::string TemplateName() const;
    CStructTemplate* BaseStruct();
    u32 AttribCount() const;
    CAttribTemplate* Attrib(u32 index);
    u32 ObjectID() const;
    u32 NumObjects() const;
    const std::list<CScriptObject*>& ObjectList() const;
    void AddObject(CScriptObject *pObject);
    void RemoveObject(CScriptObject *pObject);
    void SortObjects();
    void SetVisible(bool Visible);
    bool IsVisible();

    void DebugPrintProperties();
};

#endif // CSCRIPTTEMPLATE_H
