#ifndef CMASTERTEMPLATE_H
#define CMASTERTEMPLATE_H

#include "CLink.h"
#include "CScriptTemplate.h"
#include "Core/Resource/EGame.h"
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
    std::map<TString, CStructTemplate*> mStructTemplates;

    std::map<u32, CScriptTemplate*> mTemplates;
    std::map<u32, SState> mStates;
    std::map<u32, SMessage> mMessages;

    struct SPropIDInfo
    {
        std::vector<TString> XMLList; // List of script/struct templates that use this ID
        std::vector<IPropertyTemplate*> PropertyList; // List of all properties that use this ID
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
    CStructTemplate* StructAtSource(const TString& rkSource);

    // Inline Accessors
    EGame Game() const              { return mGame; }
    u32 NumGameVersions() const     { return mGameVersions.empty() ? 1 : mGameVersions.size(); }
    u32 NumScriptTemplates() const  { return mTemplates.size(); }
    u32 NumStates() const           { return mStates.size(); }
    u32 NumMessages() const         { return mMessages.size(); }
    bool IsLoadedSuccessfully()     { return mFullyLoaded; }
    TString GetDirectory() const    { return mSourceFile.GetFileDirectory(); }

    // Static
    static CMasterTemplate* MasterForGame(EGame Game);
    static std::list<CMasterTemplate*> MasterList();
    static TString PropertyName(u32 PropertyID);
    static u32 CreatePropertyID(IPropertyTemplate *pTemp);
    static void AddProperty(IPropertyTemplate *pTemp, const TString& rkTemplateName = "");
    static void RenameProperty(IPropertyTemplate *pTemp, const TString& rkNewName);
    static std::vector<TString> XMLsUsingID(u32 ID);
    static const std::vector<IPropertyTemplate*>* TemplatesWithMatchingID(IPropertyTemplate *pTemp);
};

#endif // CMASTERTEMPLATE_H
