#ifndef CGAMETEMPLATE_H
#define CGAMETEMPLATE_H

#include "CLink.h"
#include "CScriptTemplate.h"
#include "Core/Resource/Script/Property/Properties.h"
#include <Common/EGame.h>
#include <Common/types.h>
#include <map>

/** Serialization aid
 *  Retro switched from using integers to fourCCs to represent IDs in several cases (states/messages, object IDs).
 *  This struct is functionally an integer but it serializes as an int for MP1 and a fourCC for MP2 and on.
 */
struct SObjId
{
    union {
        u32 ID;
        CFourCC ID_4CC;
    };

    inline SObjId()                             {}
    inline SObjId(u32 InID)     : ID(InID)      {}
    inline SObjId(CFourCC InID) : ID_4CC(InID)  {}

    inline operator u32() const     { return ID; }
    inline operator CFourCC() const { return ID_4CC; }

    void Serialize(IArchive& Arc)
    {
        if (Arc.Game() <= EGame::Prime)
            Arc.SerializePrimitive(ID, SH_HexDisplay);
        else
            Arc.SerializePrimitive(ID_4CC, 0);
    }
};

/** Struct holding a reference to a script object template */
struct SScriptTemplatePath
{
    /** File path to the template file, relative to the game directory */
    TString Path;

    /** Template in memory */
    std::shared_ptr<CScriptTemplate> pTemplate;

    /** Constructor */
    SScriptTemplatePath()
    {}

    SScriptTemplatePath(const TString& kInPath, CScriptTemplate* pInTemplate)
        : Path(kInPath)
        , pTemplate( std::shared_ptr<CScriptTemplate>(pInTemplate) )
    {}

    /** Serializer */
    void Serialize(IArchive& Arc)
    {
        if (Arc.FileVersion() == 0)
        {
            Arc << SerialParameter("Path", Path, SH_Attribute);
        }
        else
        {
            Arc.SerializePrimitive(Path, 0);
        }
    }
};

/** Struct holding a reference to a property template */
struct SPropertyTemplatePath
{
    /** File path to the template file, relative to the game directory */
    TString Path;

    /** Template in memory */
    std::shared_ptr<IProperty> pTemplate;

    /** Constructor */
    SPropertyTemplatePath()
    {}

    SPropertyTemplatePath(const TString& kInPath, IProperty* pInTemplate)
        : Path(kInPath)
        , pTemplate( std::shared_ptr<IProperty>(pInTemplate) )
    {}

    /** Serializer */
    void Serialize(IArchive& Arc)
    {
        Arc << SerialParameter("Path", Path, SH_Attribute);
    }
};

/** CGameTemplate - Per-game template data */
class CGameTemplate
{
    friend class CTemplateLoader;
    friend class CTemplateWriter;

    EGame mGame;
    TString mSourceFile;
    bool mFullyLoaded;
    bool mDirty;

    /** Template arrays */
    std::map<SObjId,  SScriptTemplatePath>    mScriptTemplates;
    std::map<TString, SPropertyTemplatePath>  mPropertyTemplates;

    std::map<SObjId, TString> mStates;
    std::map<SObjId, TString> mMessages;

    /** Internal function for loading a property template from a file. */
    void Internal_LoadPropertyTemplate(SPropertyTemplatePath& Path);

public:
    CGameTemplate();
    void Serialize(IArchive& Arc);
    void Load(const TString& kFilePath);
    void Save();
    void SaveGameTemplates(bool ForceAll = false);

    u32 GameVersion(TString VersionName);
    CScriptTemplate* TemplateByID(u32 ObjectID);
    CScriptTemplate* TemplateByID(const CFourCC& ObjectID);
    CScriptTemplate* TemplateByIndex(u32 Index);
    SState StateByID(u32 StateID);
    SState StateByID(const CFourCC& StateID);
    SState StateByIndex(u32 Index);
    SMessage MessageByID(u32 MessageID);
    SMessage MessageByID(const CFourCC& MessageID);
    SMessage MessageByIndex(u32 Index);
    IProperty* FindPropertyArchetype(const TString& kTypeName);
    TString GetPropertyArchetypeFilePath(const TString& kTypeName);
    TString GetGameDirectory() const;

    // Inline Accessors
    inline EGame Game() const               { return mGame; }
    inline u32 NumScriptTemplates() const   { return mScriptTemplates.size(); }
    inline u32 NumStates() const            { return mStates.size(); }
    inline u32 NumMessages() const          { return mMessages.size(); }
    inline bool IsLoadedSuccessfully()      { return mFullyLoaded; }
};

#endif // CGAMETEMPLATE_H
