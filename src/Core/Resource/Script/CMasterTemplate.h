#ifndef CMASTERTEMPLATE_H
#define CMASTERTEMPLATE_H

#include "CLink.h"
#include "CScriptTemplate.h"
#include "Core/Resource/Script/Property/Properties.h"
#include <Common/EGame.h>
#include <Common/types.h>
#include <map>

class CMasterTemplate
{
    friend class CTemplateLoader;
    friend class CTemplateWriter;

    EGame mGame;
    TString mGameName;
    TString mSourceFile;
    u32 mVersion;
    bool mFullyLoaded;

    std::vector<TString> mGameVersions;
    std::map<TString, CStructPropertyNew*> mStructTemplates;
    std::map<TString, CEnumProperty*> mEnumTemplates;
    std::map<TString, CFlagsProperty*> mFlagsTemplates;

    std::map<u32, CScriptTemplate*> mTemplates;
    std::map<u32, SState> mStates;
    std::map<u32, SMessage> mMessages;

    struct SPropIDInfo
    {
        std::vector<TString> XMLList; // List of script/struct templates that use this ID
        std::vector<IPropertyNew*> PropertyList; // List of all properties that use this ID
    };
    static std::map<u32, SPropIDInfo> smIDMap;
    static std::map<EGame, CMasterTemplate*> smMasterMap;
    static std::map<u32, TString> smPropertyNames;
    static u32 smGameListVersion;

public:
    CMasterTemplate();
    ~CMasterTemplate();
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
    CStructPropertyNew* StructAtSource(const TString& rkSource);

    // Inline Accessors
    inline EGame Game() const               { return mGame; }
    inline TString GameName() const         { return mGameName; }
    inline u32 NumGameVersions() const      { return mGameVersions.empty() ? 1 : mGameVersions.size(); }
    inline u32 NumScriptTemplates() const   { return mTemplates.size(); }
    inline u32 NumStates() const            { return mStates.size(); }
    inline u32 NumMessages() const          { return mMessages.size(); }
    inline bool IsLoadedSuccessfully()      { return mFullyLoaded; }
    inline TString GetDirectory() const     { return mSourceFile.GetFileDirectory(); }

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
