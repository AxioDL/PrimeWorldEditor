#ifndef CMASTERTEMPLATE_H
#define CMASTERTEMPLATE_H

#include "CScriptTemplate.h"
#include "SLink.h"
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
    EGame GetGame();
    u32 NumGameVersions();
    u32 GetGameVersion(TString VersionName);
    u32 NumScriptTemplates();
    u32 NumStates();
    u32 NumMessages();
    CScriptTemplate* TemplateByID(u32 ObjectID);
    CScriptTemplate* TemplateByID(const CFourCC& ObjectID);
    CScriptTemplate* TemplateByIndex(u32 Index);
    SState StateByID(u32 StateID);
    SState StateByID(const CFourCC& StateID);
    SState StateByIndex(u32 Index);
    SMessage MessageByID(u32 MessageID);
    SMessage MessageByID(const CFourCC& MessageID);
    SMessage MessageByIndex(u32 Index);
    TString GetDirectory() const;
    CStructTemplate* GetStructAtSource(const TString& rkSource);
    bool IsLoadedSuccessfully();

    static CMasterTemplate* GetMasterForGame(EGame Game);
    static std::list<CMasterTemplate*> GetMasterList();
    static TString GetPropertyName(u32 PropertyID);
    static u32 CreatePropertyID(IPropertyTemplate *pTemp);
    static void AddProperty(IPropertyTemplate *pTemp, const TString& rkTemplateName = "");
    static void RenameProperty(IPropertyTemplate *pTemp, const TString& rkNewName);
    static std::vector<TString> GetXMLsUsingID(u32 ID);
    static const std::vector<IPropertyTemplate*>* GetTemplatesWithMatchingID(IPropertyTemplate *pTemp);
};

// ************ INLINE ************
inline u32 CMasterTemplate::NumScriptTemplates() {
    return mTemplates.size();
}

inline u32 CMasterTemplate::NumStates() {
    return mStates.size();
}

inline u32 CMasterTemplate::NumMessages() {
    return mMessages.size();
}

#endif // CMASTERTEMPLATE_H
