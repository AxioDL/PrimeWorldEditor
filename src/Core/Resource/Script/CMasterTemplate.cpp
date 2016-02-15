#include "CMasterTemplate.h"
#include "Core/Resource/Factory/CWorldLoader.h"
#include <Common/Log.h>

CMasterTemplate::CMasterTemplate()
{
    mVersion = 0;
    mFullyLoaded = false;
}

CMasterTemplate::~CMasterTemplate()
{
    for (auto it = mTemplates.begin(); it != mTemplates.end(); it++)
        delete it->second;
}

EGame CMasterTemplate::GetGame()
{
    return mGame;
}

u32 CMasterTemplate::NumGameVersions()
{
    if (mGameVersions.empty()) return 1;
    else return mGameVersions.size();
}

u32 CMasterTemplate::GetGameVersion(TString VersionName)
{
    VersionName = VersionName.ToLower();

    for (u32 iVer = 0; iVer < mGameVersions.size(); iVer++)
        if (mGameVersions[iVer].ToLower() == VersionName)
            return iVer;

    return -1;
}

CScriptTemplate* CMasterTemplate::TemplateByID(u32 ObjectID)
{
    auto it = mTemplates.find(ObjectID);

    if (it != mTemplates.end())
        return it->second;
    else
        return nullptr;
}

CScriptTemplate* CMasterTemplate::TemplateByID(const CFourCC& ObjectID)
{
    return TemplateByID(ObjectID.ToLong());
}

CScriptTemplate* CMasterTemplate::TemplateByIndex(u32 Index)
{
    auto it = mTemplates.begin();
    return (std::next(it, Index))->second;
}

TString CMasterTemplate::StateByID(u32 StateID)
{
    auto it = mStates.find(StateID);

    if (it != mStates.end())
        return it->second;
    else
        return "Invalid";
}

TString CMasterTemplate::StateByID(const CFourCC& State)
{
    return StateByID(State.ToLong());
}

TString CMasterTemplate::StateByIndex(u32 Index)
{
    auto it = mStates.begin();
    return (std::next(it, Index))->second;
}

TString CMasterTemplate::MessageByID(u32 MessageID)
{
    auto it = mMessages.find(MessageID);

    if (it != mMessages.end())
        return it->second;
    else
        return "Invalid";
}

TString CMasterTemplate::MessageByID(const CFourCC& MessageID)
{
    return MessageByID(MessageID.ToLong());
}

TString CMasterTemplate::MessageByIndex(u32 Index)
{
    auto it = mMessages.begin();
    return (std::next(it, Index))->second;
}

TString CMasterTemplate::GetDirectory() const
{
    return mSourceFile.GetFileDirectory();
}

CStructTemplate* CMasterTemplate::GetStructAtSource(const TString& rkSource)
{
    auto InfoIt = mStructTemplates.find(rkSource);

    if (InfoIt != mStructTemplates.end())
        return InfoIt->second;

    else return nullptr;
}

bool CMasterTemplate::IsLoadedSuccessfully()
{
    return mFullyLoaded;
}

// ************ STATIC ************
CMasterTemplate* CMasterTemplate::GetMasterForGame(EGame Game)
{
    auto it = smMasterMap.find(Game);

    if (it != smMasterMap.end())
        return it->second;
    else
        return nullptr;
}

std::list<CMasterTemplate*> CMasterTemplate::GetMasterList()
{
    std::list<CMasterTemplate*> list;

    for (auto it = smMasterMap.begin(); it != smMasterMap.end(); it++)
        list.push_back(it->second);

    return list;
}

TString CMasterTemplate::GetPropertyName(u32 PropertyID)
{
    auto it = smPropertyNames.find(PropertyID);

    if (it != smPropertyNames.end())
        return it->second;
    else
        return "Unknown";
}

u32 CMasterTemplate::CreatePropertyID(IPropertyTemplate *pTemp)
{
    // MP1 properties don't have IDs so we can use this function to create one to track instances of a particular property.
    // To ensure the IDs are unique we'll create a hash using two things: the struct source file and the ID string (relative to the struct).
    TString IDString = pTemp->IDString(false);
    TString Source;
    CStructTemplate *pStruct = pTemp->Parent();

    while (pStruct)
    {
        Source = pStruct->SourceFile();
        if (!Source.IsEmpty()) break;
        IDString.Prepend(pStruct->IDString(false) + ":");
        pStruct = pStruct->Parent();
    }

    return IDString.Hash32() * Source.Hash32();
}

void CMasterTemplate::AddProperty(IPropertyTemplate *pTemp, const TString& rkTemplateName /*= ""*/)
{
    u32 ID;

    if (pTemp->Game() >= eEchoesDemo)
        ID = pTemp->PropertyID();

    // Use a different ID for MP1
    else
    {
        // For MP1 we only really need to track properties that come from struct templates.
        if (!pTemp->IsFromStructTemplate()) return;
        else ID = CreatePropertyID(pTemp);
    }

    auto it = smIDMap.find(ID);

    // Add this property/template to existing ID info
    if (it != smIDMap.end())
    {
        SPropIDInfo& rInfo = it->second;
        rInfo.PropertyList.push_back(pTemp);

        if (!rkTemplateName.IsEmpty())
        {
            bool NewTemplate = true;

            for (u32 iTemp = 0; iTemp < rInfo.XMLList.size(); iTemp++)
            {
                if (rInfo.XMLList[iTemp] == rkTemplateName)
                {
                    NewTemplate = false;
                    break;
                }
            }

            if (NewTemplate)
                rInfo.XMLList.push_back(rkTemplateName);
        }
    }

    // Create new ID info
    else
    {
        SPropIDInfo Info;
        if (!rkTemplateName.IsEmpty()) Info.XMLList.push_back(rkTemplateName);
        Info.PropertyList.push_back(pTemp);
        smIDMap[ID] = Info;
    }
}

void CMasterTemplate::RenameProperty(IPropertyTemplate *pTemp, const TString& rkNewName)
{
    u32 ID = pTemp->PropertyID();
    if (ID <= 0xFF) ID = CreatePropertyID(pTemp);

    // Master name list
    auto NameIt = smPropertyNames.find(ID);
    TString Original;

    if (NameIt != smPropertyNames.end())
    {
        Original = NameIt->second;
        smPropertyNames[ID] = rkNewName;
    }

    // Properties
    auto InfoIt = smIDMap.find(ID);

    if (InfoIt != smIDMap.end())
    {
        const SPropIDInfo& rkInfo = InfoIt->second;

        for (u32 iTemp = 0; iTemp < rkInfo.PropertyList.size(); iTemp++)
        {
            if (Original.IsEmpty() || rkInfo.PropertyList[iTemp]->Name() == Original)
                rkInfo.PropertyList[iTemp]->SetName(rkNewName);
        }
    }
}

std::vector<TString> CMasterTemplate::GetXMLsUsingID(u32 ID)
{
    auto InfoIt = smIDMap.find(ID);

    if (InfoIt != smIDMap.end())
    {
        const SPropIDInfo& rkInfo = InfoIt->second;
        return rkInfo.XMLList;
    }
    else
        return std::vector<TString>();
}

const std::vector<IPropertyTemplate*>* CMasterTemplate::GetTemplatesWithMatchingID(IPropertyTemplate *pTemp)
{
    u32 ID = pTemp->PropertyID();
    if (ID <= 0xFF) ID = CreatePropertyID(pTemp);

    auto InfoIt = smIDMap.find(ID);

    if (InfoIt != smIDMap.end())
    {
        const SPropIDInfo& rkInfo = InfoIt->second;
        return &rkInfo.PropertyList;
    }
    return nullptr;
}

std::map<u32, CMasterTemplate::SPropIDInfo> CMasterTemplate::smIDMap;
std::map<EGame, CMasterTemplate*> CMasterTemplate::smMasterMap;
std::map<u32, TString> CMasterTemplate::smPropertyNames;
u32 CMasterTemplate::smGameListVersion;
