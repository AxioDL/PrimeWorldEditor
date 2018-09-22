#include "CMasterTemplate.h"
#include "Core/Resource/Factory/CWorldLoader.h"
#include <Common/Log.h>

CMasterTemplate::CMasterTemplate()
    : mFullyLoaded(false)
{
}

void CMasterTemplate::Serialize(IArchive& Arc)
{
    Arc << SerialParameter("ScriptObjects", mScriptTemplates)
        << SerialParameter("PropertyArchetypes", mPropertyTemplates)
        << SerialParameter("States", mStates)
        << SerialParameter("Messages", mMessages);
}

void CMasterTemplate::LoadSubTemplates()
{
    for (auto Iter = mScriptTemplates.begin(); Iter != mScriptTemplates.end(); Iter++)
        Internal_LoadScriptTemplate( Iter->second );

    for (auto Iter = mPropertyTemplates.begin(); Iter != mPropertyTemplates.end(); Iter++)
        Internal_LoadPropertyTemplate( Iter->second );
}

void CMasterTemplate::Internal_LoadScriptTemplate(SScriptTemplatePath& Path)
{
    ASSERT(Path.pTemplate == nullptr); // make sure it hasn't been loaded yet

    const TString kGameDir = GetGameDirectory(true);
    const TString kTemplateFilePath = kGameDir + Path.Path;
    CXMLReader Reader(kTemplateFilePath);
    ASSERT(Reader.IsValid());

    Path.pTemplate = std::make_shared<CScriptTemplate>(this, Path.ID.ID, Path.Path);
    Path.pTemplate->Serialize(Reader);
    Path.pTemplate->PostLoad();
}

void CMasterTemplate::Internal_LoadPropertyTemplate(SPropertyTemplatePath& Path)
{
    if (Path.pTemplate != nullptr) // don't load twice
        return;

    const TString kGameDir = GetGameDirectory(true);
    const TString kTemplateFilePath = kGameDir + Path.Path;
    CXMLReader Reader(kTemplateFilePath);
    ASSERT(Reader.IsValid());

    Reader << SerialParameter("PropertyArchetype", Path.pTemplate);
    ASSERT(Path.pTemplate != nullptr);

    Path.pTemplate->SetPropertyFlags( EPropertyFlag::IsArchetype );
    Path.pTemplate->Initialize(nullptr, nullptr, 0);
}

void CMasterTemplate::SaveSubTemplates()
{
    const TString kGameDir = GetGameDirectory(true);

    for (auto Iter = mScriptTemplates.begin(); Iter != mScriptTemplates.end(); Iter++)
    {
        SScriptTemplatePath& Path = Iter->second;
        TString OutPath = kGameDir + Path.Path;

        FileUtil::MakeDirectory( OutPath.GetFileDirectory() );
        CXMLWriter Writer(OutPath, "ScriptObject", 0, Game());
        Path.pTemplate->Serialize(Writer);
    }

    for (auto Iter = mPropertyTemplates.begin(); Iter != mPropertyTemplates.end(); Iter++)
    {
        SPropertyTemplatePath& Path = Iter->second;
        TString OutPath = kGameDir + Path.Path;

        FileUtil::MakeDirectory( OutPath.GetFileDirectory() );
        CXMLWriter Writer(OutPath, "PropertyTemplate", 0, Game());
        Writer << SerialParameter("PropertyArchetype", Path.pTemplate);
    }
}

u32 CMasterTemplate::GameVersion(TString VersionName)
{
    return -1;
}

CScriptTemplate* CMasterTemplate::TemplateByID(u32 ObjectID)
{
    auto it = mScriptTemplates.find(ObjectID);

    if (it != mScriptTemplates.end())
        return it->second.pTemplate.get();
    else
        return nullptr;
}

CScriptTemplate* CMasterTemplate::TemplateByID(const CFourCC& ObjectID)
{
    return TemplateByID(ObjectID.ToLong());
}

CScriptTemplate* CMasterTemplate::TemplateByIndex(u32 Index)
{
    auto it = mScriptTemplates.begin();
    return (std::next(it, Index))->second.pTemplate.get();
}

SState CMasterTemplate::StateByID(u32 StateID)
{
    auto Iter = mStates.find(StateID);

    if (Iter != mStates.end())
        return SState(Iter->first, Iter->second);
    else
        return SState(-1, "Invalid");
}

SState CMasterTemplate::StateByID(const CFourCC& State)
{
    return StateByID(State.ToLong());
}

SState CMasterTemplate::StateByIndex(u32 Index)
{
    auto Iter = mStates.begin();
    Iter = std::next(Iter, Index);
    return SState(Iter->first, Iter->second);
}

SMessage CMasterTemplate::MessageByID(u32 MessageID)
{
    auto Iter = mMessages.find(MessageID);

    if (Iter != mMessages.end())
        return SMessage(Iter->first, Iter->second);
    else
        return SMessage(-1, "Invalid");
}

SMessage CMasterTemplate::MessageByID(const CFourCC& MessageID)
{
    return MessageByID(MessageID.ToLong());
}

SMessage CMasterTemplate::MessageByIndex(u32 Index)
{
    auto Iter = mMessages.begin();
    Iter = std::next(Iter, Index);
    return SMessage(Iter->first, Iter->second);
}

IProperty* CMasterTemplate::FindPropertyArchetype(const TString& kTypeName)
{
    auto Iter = mPropertyTemplates.find(kTypeName);

    // Should require Iter to be valid in the future. For now, this is possible for some of the transition template loader code.
//    ASSERT(Iter != mPropertyTemplates.end()); // Requested archetype property does not exist; missing or malformed template
    if (Iter == mPropertyTemplates.end())
    {
        return nullptr;
    }

    // If the template isn't loaded yet, then load it.
    // This has to be done here to allow recursion while loading other property archetypes, because some properties may
    // request archetypes of other properties that haven't been loaded yet during their load.
    SPropertyTemplatePath& Path = Iter->second;
    if (!Path.pTemplate)
    {
        Internal_LoadPropertyTemplate(Path);
        ASSERT(Path.pTemplate != nullptr); // Load failed; missing or malformed template
    }

    return Path.pTemplate.get();
}

TString CMasterTemplate::GetGameDirectory(bool Absolute) const
{
    TString Out = mSourceFile.GetFileDirectory();
    return Absolute ? "../templates_new/" + Out : Out;
}

// ************ STATIC ************
CMasterTemplate* CMasterTemplate::MasterForGame(EGame Game)
{
    auto it = smMasterMap.find(Game);

    if (it != smMasterMap.end())
        return it->second;
    else
        return nullptr;
}

std::list<CMasterTemplate*> CMasterTemplate::MasterList()
{
    std::list<CMasterTemplate*> list;

    for (auto it = smMasterMap.begin(); it != smMasterMap.end(); it++)
        list.push_back(it->second);

    return list;
}

TString CMasterTemplate::FindGameName(EGame Game)
{
    CMasterTemplate *pMaster = MasterForGame(Game);
    return pMaster ? pMaster->GameName() : "Unknown Game";
}

EGame CMasterTemplate::FindGameForName(const TString& rkName)
{
    std::list<CMasterTemplate*> Masters = MasterList();

    for (auto It = Masters.begin(); It != Masters.end(); It++)
    {
        CMasterTemplate *pMaster = *It;
        if (pMaster->GameName() == rkName)
            return pMaster->Game();
    }

    return eUnknownGame;
}

TString CMasterTemplate::PropertyName(u32 PropertyID)
{
    auto it = smPropertyNames.find(PropertyID);

    if (it != smPropertyNames.end())
        return it->second;
    else
        return "Unknown";
}

// Removing these functions for now. I'm not sure of the best way to go about implementing them under the new system yet.
u32 CMasterTemplate::CreatePropertyID(IProperty* pProp)
{
    // MP1 properties don't have IDs so we can use this function to create one to track instances of a particular property.
    // To ensure the IDs are unique we'll create a hash using two things: the struct source file and the ID string (relative to the struct).
    //
    // Note for properties that have accurate names we can apply a CRC32 to the name to generate a hash equivalent to what the hash would
    // have been if this were an MP2/3 property. In an ideal world where every property was named, this would be great. However, we have a
    // lot of properties that have generic names like "Unknown", and they should be tracked separately as they are in all likelihood
    // different properties. So for this reason, we only want to track sub-instances of one property under one ID.
    TString IDString = pProp->Archetype()->IDString(true);
    TString TemplateFile = pProp->GetTemplateFileName();

    CCRC32 Hash;
    Hash.Hash(*IDString);
    Hash.Hash(*TemplateFile);
    return Hash.Digest();
}

void CMasterTemplate::AddProperty(IProperty* pProp, const TString& rkTemplateName /*= ""*/)
{
    u32 ID;

    if (pProp->Game() >= eEchoesDemo)
        ID = pProp->ID();

    // Use a different ID for MP1
    else
    {
        // For MP1 we only really need to track properties that come from struct templates.
        IProperty* pArchetype = pProp->Archetype();

        if (!pArchetype ||
             pArchetype->ScriptTemplate() != nullptr ||
             pArchetype->RootParent()->Type() != EPropertyType::Struct)
            return;

        ID = CreatePropertyID(pProp);
    }

    auto it = smIDMap.find(ID);

    // Add this property/template to existing ID info
    if (it != smIDMap.end())
    {
        SPropIDInfo& rInfo = it->second;
        rInfo.PropertyList.push_back(pProp);

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
        Info.PropertyList.push_back(pProp);
        smIDMap[ID] = Info;
    }
}

void CMasterTemplate::RenameProperty(IProperty* pProp, const TString& rkNewName)
{
    u32 ID = pProp->ID();
    if (ID <= 0xFF) ID = CreatePropertyID(pProp);
    RenameProperty(ID, rkNewName);
}

void CMasterTemplate::RenameProperty(u32 ID, const TString& rkNewName)
{
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

        for (u32 PropertyIdx = 0; PropertyIdx < rkInfo.PropertyList.size(); PropertyIdx++)
        {
            if (Original.IsEmpty() || rkInfo.PropertyList[PropertyIdx]->Name() == Original)
                rkInfo.PropertyList[PropertyIdx]->SetName(rkNewName);
        }
    }
}

void CMasterTemplate::XMLsUsingID(u32 ID, std::vector<TString>& rOutList)
{
    auto InfoIt = smIDMap.find(ID);

    if (InfoIt != smIDMap.end())
    {
        const SPropIDInfo& rkInfo = InfoIt->second;
        rOutList = rkInfo.XMLList;
    }
}

const std::vector<IProperty*>* CMasterTemplate::TemplatesWithMatchingID(IProperty* pProp)
{
    u32 ID = pProp->ID();
    if (ID <= 0xFF) ID = CreatePropertyID(pProp);

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
