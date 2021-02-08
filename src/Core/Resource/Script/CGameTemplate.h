#ifndef CGAMETEMPLATE_H
#define CGAMETEMPLATE_H

#include "CLink.h"
#include "CScriptTemplate.h"
#include "Core/Resource/Script/Property/Properties.h"
#include <Common/BasicTypes.h>
#include <Common/EGame.h>
#include <map>

/** Serialization aid
 *  Retro switched from using integers to fourCCs to represent IDs in several cases (states/messages, object IDs).
 *  This struct is functionally an integer but it serializes as an int for MP1 and a fourCC for MP2 and on.
 */
struct SObjId
{
    union {
        uint32 ID;
        CFourCC ID_4CC;
    };

    SObjId()                             {}
    SObjId(uint32 InID)  : ID(InID)      {}
    SObjId(CFourCC InID) : ID_4CC(InID)  {}

    operator uint32() const  { return ID; }
    operator CFourCC() const { return ID_4CC; }

    void Serialize(IArchive& Arc)
    {
        if (Arc.Game() <= EGame::Prime)
            Arc.SerializePrimitive(ID, SH_HexDisplay);
        else
            Arc.SerializePrimitive(ID_4CC, 0);
    }
};

/** Struct holding a reference to a template */
template<typename TemplateT>
struct TTemplatePath
{
    /** File path to the template file, relative to the game directory */
    TString Path;

    /** Template in memory */
    std::shared_ptr<TemplateT> pTemplate;

    /** Constructor */
    TTemplatePath()
    {}

    TTemplatePath(const TString& kInPath, TemplateT* pInTemplate)
        : Path(kInPath)
        , pTemplate( std::shared_ptr<TemplateT>(pInTemplate) )
    {}

    /** Serializer */
    void Serialize(IArchive& Arc)
    {
        Arc << SerialParameter("Path", Path, SH_Attribute);
    }
};

using SScriptTemplatePath = TTemplatePath<CScriptTemplate>;
using SPropertyTemplatePath = TTemplatePath<IProperty>;

/** CGameTemplate - Per-game template data */
class CGameTemplate
{
    EGame mGame{};
    TString mSourceFile;
    bool mFullyLoaded = false;
    bool mDirty = false;

    /** Template arrays */
    std::map<SObjId,  SScriptTemplatePath>    mScriptTemplates;
    std::map<TString, SPropertyTemplatePath>  mPropertyTemplates;
    std::map<TString, SScriptTemplatePath>    mMiscTemplates;

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

    uint32 GameVersion(TString VersionName);
    CScriptTemplate* TemplateByID(uint32 ObjectID);
    CScriptTemplate* TemplateByID(const CFourCC& ObjectID);
    CScriptTemplate* TemplateByIndex(uint32 Index);
    SState StateByID(uint32 StateID) const;
    SState StateByID(const CFourCC& StateID) const;
    SState StateByIndex(uint32 Index) const;
    SMessage MessageByID(uint32 MessageID) const;
    SMessage MessageByID(const CFourCC& MessageID) const;
    SMessage MessageByIndex(uint32 Index) const;
    IProperty* FindPropertyArchetype(const TString& kTypeName);
    TString GetPropertyArchetypeFilePath(const TString& kTypeName) const;
    bool RenamePropertyArchetype(const TString& kTypeName, const TString& kNewTypeName);
    CScriptTemplate* FindMiscTemplate(const TString& kTemplateName);
    TString GetGameDirectory() const;

    // Accessors
    EGame Game() const                   { return mGame; }
    uint32 NumScriptTemplates() const    { return mScriptTemplates.size(); }
    uint32 NumStates() const             { return mStates.size(); }
    uint32 NumMessages() const           { return mMessages.size(); }
    bool IsLoadedSuccessfully() const    { return mFullyLoaded; }
};

#endif // CGAMETEMPLATE_H
