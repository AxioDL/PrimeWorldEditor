#ifndef CMASTERTEMPLATE_H
#define CMASTERTEMPLATE_H

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
        if (Arc.Game() <= ePrime)
            Arc.SerializePrimitive(ID, SH_HexDisplay);
        else
            Arc.SerializePrimitive(ID_4CC, 0);
    }
};

class CMasterTemplate
{
    friend class CTemplateLoader;
    friend class CTemplateWriter;

    /** Struct holding a reference to a script object template */
    struct SScriptTemplatePath
    {
        /** Script object ID */
        SObjId ID;

        /** File path to the template file, relative to the game directory */
        TString Path;

        /** Template in memory */
        std::shared_ptr<CScriptTemplate> pTemplate;

        /** Constructor */
        SScriptTemplatePath()
            : ID(0)
        {}

        SScriptTemplatePath(u32 InID, const TString& kInPath, CScriptTemplate* pInTemplate)
            : ID(InID)
            , Path(kInPath)
            , pTemplate( std::shared_ptr<CScriptTemplate>(pInTemplate) )
        {}

        SScriptTemplatePath(const CFourCC& kInID, const TString& kInPath, CScriptTemplate* pInTemplate)
            : ID(kInID)
            , Path(kInPath)
            , pTemplate( std::shared_ptr<CScriptTemplate>(pInTemplate) )
        {}

        /** Serializer */
        void Serialize(IArchive& Arc)
        {
            Arc << SerialParameter("ID", ID, SH_Attribute)
                << SerialParameter("Path", Path, SH_Attribute);
        }
    };

    /** Struct holding a reference to a property template */
    struct SPropertyTemplatePath
    {
        /** File path to the template file, relative to the game directory */
        TString Path;

        /** Template in memory */
        std::shared_ptr<IPropertyNew> pTemplate;

        /** Constructor */
        SPropertyTemplatePath()
        {}

        SPropertyTemplatePath(const TString& kInPath, IPropertyNew* pInTemplate)
            : Path(kInPath)
            , pTemplate( std::shared_ptr<IPropertyNew>(pInTemplate) )
        {}

        /** Serializer */
        void Serialize(IArchive& Arc)
        {
            Arc << SerialParameter("Path", Path, SH_Attribute);
        }
    };

    EGame mGame;
    TString mGameName;
    TString mSourceFile;
    bool mFullyLoaded;

    /** Template arrays */
    std::map<SObjId,  SScriptTemplatePath>    mScriptTemplates;
    std::map<TString, SPropertyTemplatePath>  mPropertyTemplates;

    std::map<SObjId, TString> mStates;
    std::map<SObjId, TString> mMessages;

    struct SPropIDInfo
    {
        std::vector<TString> XMLList; // List of script/struct templates that use this ID
        std::vector<IPropertyNew*> PropertyList; // List of all properties that use this ID
    };
    static std::map<u32, SPropIDInfo> smIDMap;
    static std::map<EGame, CMasterTemplate*> smMasterMap;
    static std::map<u32, TString> smPropertyNames;
    static u32 smGameListVersion;

    void Internal_LoadScriptTemplate(SScriptTemplatePath& Path);
    void Internal_LoadPropertyTemplate(SPropertyTemplatePath& Path);

public:
    CMasterTemplate();
    void Serialize(IArchive& Arc);
    void LoadSubTemplates();
    void SaveSubTemplates();
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
    IPropertyNew* FindPropertyArchetype(const TString& kTypeName);
    TString GetGameDirectory(bool Absolute = false) const;

    // Inline Accessors
    inline EGame Game() const               { return mGame; }
    inline TString GameName() const         { return mGameName; }
    inline u32 NumScriptTemplates() const   { return mScriptTemplates.size(); }
    inline u32 NumStates() const            { return mStates.size(); }
    inline u32 NumMessages() const          { return mMessages.size(); }
    inline bool IsLoadedSuccessfully()      { return mFullyLoaded; }

    // Static
    static CMasterTemplate* MasterForGame(EGame Game);
    static std::list<CMasterTemplate*> MasterList();
    static TString FindGameName(EGame Game);
    static EGame FindGameForName(const TString& rkName);
    static TString PropertyName(u32 PropertyID);
    static u32 CreatePropertyID(IPropertyNew *pTemp);
    static void AddProperty(IPropertyNew *pTemp, const TString& rkTemplateName = "");
    static void RenameProperty(IPropertyNew *pTemp, const TString& rkNewName);
    static void RenameProperty(u32 ID, const TString& rkNewName);
    static void XMLsUsingID(u32 ID, std::vector<TString>& rOutList);
    static const std::vector<IPropertyNew*>* TemplatesWithMatchingID(IPropertyNew *pTemp);
};

#endif // CMASTERTEMPLATE_H
