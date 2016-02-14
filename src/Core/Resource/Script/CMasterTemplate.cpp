#include "CMasterTemplate.h"
#include "Core/Resource/Factory/CWorldLoader.h"
#include "Core/Log.h"

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

void CMasterTemplate::AddProperty(IPropertyTemplate *pTemp, const TString& rkTemplateName)
{
    auto it = smIDMap.find(pTemp->PropertyID());

    // Add this property/template to existing ID info
    if (it != smIDMap.end())
    {
        SPropIDInfo& rInfo = it->second;
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

        it->second.PropertyList.push_back(pTemp);
    }

    // Create new ID info
    else
    {
        SPropIDInfo Info;
        Info.XMLList.push_back(rkTemplateName);
        Info.PropertyList.push_back(pTemp);
        smIDMap[pTemp->PropertyID()] = Info;
    }
}

void CMasterTemplate::RenameProperty(u32 ID, const TString& rkNewName)
{
    auto NameIt = smPropertyNames.find(ID);
    TString CurName = (NameIt == smPropertyNames.end() ? "" : NameIt->second);

    auto InfoIt = smIDMap.find(ID);

    if (InfoIt != smIDMap.end())
    {
        const SPropIDInfo& rkInfo = InfoIt->second;

        for (u32 iTemp = 0; iTemp < rkInfo.PropertyList.size(); iTemp++)
        {
            IPropertyTemplate *pTemp = rkInfo.PropertyList[iTemp];

            if (pTemp->Name() == CurName)
                pTemp->SetName(rkNewName);
        }
    }

    if (NameIt != smPropertyNames.end())
        smPropertyNames[ID] = rkNewName;
}

std::vector<TString> CMasterTemplate::GetTemplatesUsingID(u32 ID)
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

std::map<u32, CMasterTemplate::SPropIDInfo> CMasterTemplate::smIDMap;
std::map<EGame, CMasterTemplate*> CMasterTemplate::smMasterMap;
std::map<u32, TString> CMasterTemplate::smPropertyNames;
u32 CMasterTemplate::smGameListVersion;
