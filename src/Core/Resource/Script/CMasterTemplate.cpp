#include "CMasterTemplate.h"
#include "../factory/CWorldLoader.h"
#include <Core/Log.h>

CMasterTemplate::CMasterTemplate()
{
    mVersion = 0;
    mFullyLoaded = false;
    mHasPropList = false;
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

CPropertyTemplate* CMasterTemplate::GetProperty(u32 PropertyID)
{
    auto it = mPropertyList.find(PropertyID);

    if (it != mPropertyList.end())
        return it->second;
    else
        return nullptr;
}

bool CMasterTemplate::HasPropertyList()
{
    return mHasPropList;
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

std::map<EGame, CMasterTemplate*> CMasterTemplate::smMasterMap;
u32 CMasterTemplate::smGameListVersion;
