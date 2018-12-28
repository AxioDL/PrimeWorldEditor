#include "CGameTemplate.h"
#include "NPropertyMap.h"
#include "Core/Resource/Factory/CWorldLoader.h"
#include <Common/Log.h>

CGameTemplate::CGameTemplate()
    : mFullyLoaded(false)
    , mDirty(false)
{
}

void CGameTemplate::Serialize(IArchive& Arc)
{
    Arc << SerialParameter("ScriptObjects", mScriptTemplates)
        << SerialParameter("PropertyArchetypes", mPropertyTemplates)
        << SerialParameter("MiscTemplates", mMiscTemplates)
        << SerialParameter("States", mStates)
        << SerialParameter("Messages", mMessages);
}

void CGameTemplate::Load(const TString& kFilePath)
{
    CXMLReader Reader(kFilePath);
    ASSERT(Reader.IsValid());

    mGame = Reader.Game();
    Serialize(Reader);

    mSourceFile = kFilePath;
    mFullyLoaded = true;

    // Load all sub-templates
    const TString gkGameRoot = GetGameDirectory();

    for (auto Iter = mScriptTemplates.begin(); Iter != mScriptTemplates.end(); Iter++)
    {
        SScriptTemplatePath& ScriptPath = Iter->second;
        TString AbsPath = gkGameRoot + ScriptPath.Path;
        ScriptPath.pTemplate = std::make_shared<CScriptTemplate>(this, Iter->first, AbsPath);
    }

    for (auto Iter = mPropertyTemplates.begin(); Iter != mPropertyTemplates.end(); Iter++)
    {
        // For properties, remember that property archetypes can reference other archetypes which
        // may not be loaded yet.. so if this happens, the referenced property will be loaded,
        // meaning property templates can be loaded out of order, so we need to make sure
        // that we don't load any template more than once.
        SPropertyTemplatePath& PropertyPath = Iter->second;

        if (!PropertyPath.pTemplate)
        {
            Internal_LoadPropertyTemplate(Iter->second);
        }
    }

    for (auto Iter = mMiscTemplates.begin(); Iter != mMiscTemplates.end(); Iter++)
    {
        SScriptTemplatePath& MiscPath = Iter->second;
        TString AbsPath = gkGameRoot + MiscPath.Path;
        MiscPath.pTemplate = std::make_shared<CScriptTemplate>(this, -1, AbsPath);
    }
}

void CGameTemplate::Save()
{
    debugf("Saving game template: %s", *mSourceFile);
    CXMLWriter Writer(mSourceFile, "Game", 0, mGame);
    ASSERT(Writer.IsValid());
    Serialize(Writer);
    mDirty = false;
}

/** Internal function for loading a property template from a file. */
void CGameTemplate::Internal_LoadPropertyTemplate(SPropertyTemplatePath& Path)
{
    if (Path.pTemplate != nullptr) // don't load twice
        return;

    const TString kGameDir = GetGameDirectory();
    const TString kTemplateFilePath = kGameDir + Path.Path;
    CXMLReader Reader(kTemplateFilePath);
    ASSERT(Reader.IsValid());

    Reader << SerialParameter("PropertyArchetype", Path.pTemplate);
    ASSERT(Path.pTemplate != nullptr);

    Path.pTemplate->Initialize(nullptr, nullptr, 0);
}

void CGameTemplate::SaveGameTemplates(bool ForceAll /*= false*/)
{
    const TString kGameDir = GetGameDirectory();

    if (mDirty || ForceAll)
    {
        Save();
    }

    for (auto Iter = mScriptTemplates.begin(); Iter != mScriptTemplates.end(); Iter++)
    {
        SScriptTemplatePath& Path = Iter->second;

        if( Path.pTemplate )
        {
            Path.pTemplate->Save(ForceAll);
        }
    }

    for (auto Iter = mPropertyTemplates.begin(); Iter != mPropertyTemplates.end(); Iter++)
    {
        SPropertyTemplatePath& Path = Iter->second;

        if( Path.pTemplate )
        {
            if( ForceAll || Path.pTemplate->IsDirty() )
            {
                const TString kOutPath = kGameDir + Path.Path;
                FileUtil::MakeDirectory( kOutPath.GetFileDirectory() );

                debugf("Saving property template: %s", *kOutPath);
                CXMLWriter Writer(kOutPath, "PropertyTemplate", 0, Game());
                ASSERT(Writer.IsValid());

                Writer << SerialParameter("PropertyArchetype", Path.pTemplate);
                Path.pTemplate->ClearDirtyFlag();
            }
        }
    }

    for (auto Iter = mMiscTemplates.begin(); Iter != mMiscTemplates.end(); Iter++)
    {
        SScriptTemplatePath& Path = Iter->second;

        if( Path.pTemplate )
        {
            Path.pTemplate->Save(ForceAll);
        }
    }
}

uint32 CGameTemplate::GameVersion(TString VersionName)
{
    return -1;
}

CScriptTemplate* CGameTemplate::TemplateByID(uint32 ObjectID)
{
    auto it = mScriptTemplates.find(ObjectID);

    if (it != mScriptTemplates.end())
        return it->second.pTemplate.get();
    else
        return nullptr;
}

CScriptTemplate* CGameTemplate::TemplateByID(const CFourCC& ObjectID)
{
    return TemplateByID(ObjectID.ToLong());
}

CScriptTemplate* CGameTemplate::TemplateByIndex(uint32 Index)
{
    auto it = mScriptTemplates.begin();
    return (std::next(it, Index))->second.pTemplate.get();
}

SState CGameTemplate::StateByID(uint32 StateID)
{
    auto Iter = mStates.find(StateID);

    if (Iter != mStates.end())
        return SState(Iter->first, Iter->second);
    else
        return SState(-1, "Invalid");
}

SState CGameTemplate::StateByID(const CFourCC& State)
{
    return StateByID(State.ToLong());
}

SState CGameTemplate::StateByIndex(uint32 Index)
{
    auto Iter = mStates.begin();
    Iter = std::next(Iter, Index);
    return SState(Iter->first, Iter->second);
}

SMessage CGameTemplate::MessageByID(uint32 MessageID)
{
    auto Iter = mMessages.find(MessageID);

    if (Iter != mMessages.end())
        return SMessage(Iter->first, Iter->second);
    else
        return SMessage(-1, "Invalid");
}

SMessage CGameTemplate::MessageByID(const CFourCC& MessageID)
{
    return MessageByID(MessageID.ToLong());
}

SMessage CGameTemplate::MessageByIndex(uint32 Index)
{
    auto Iter = mMessages.begin();
    Iter = std::next(Iter, Index);
    return SMessage(Iter->first, Iter->second);
}

IProperty* CGameTemplate::FindPropertyArchetype(const TString& kTypeName)
{
    auto Iter = mPropertyTemplates.find(kTypeName);

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

TString CGameTemplate::GetPropertyArchetypeFilePath(const TString& kTypeName)
{
    auto Iter = mPropertyTemplates.find(kTypeName);
    ASSERT(Iter != mPropertyTemplates.end());
    return GetGameDirectory() + Iter->second.Path;
}

bool CGameTemplate::RenamePropertyArchetype(const TString& kTypeName, const TString& kNewTypeName)
{
    if( kTypeName != kNewTypeName )
    {
        // Fetch the property that we are going to be renaming.
        // Validate type, too, because we only support renaming struct archetypes at the moment
        auto Iter = mPropertyTemplates.find(kTypeName);

        if( Iter != mPropertyTemplates.end() )
        {
            SPropertyTemplatePath& Path = Iter->second;
            IProperty* pArchetype = Path.pTemplate.get();

            if( pArchetype )
            {
                // Attempt to move the XML to the new location.
                TString OldPath = GetGameDirectory() + Path.Path;
                TString NewPath = OldPath.GetFileDirectory() + kNewTypeName + ".xml";

                if( FileUtil::MoveFile(OldPath, NewPath) )
                {
                    // Update the name in the game template's internal mapping
                    TString RelativePath = FileUtil::MakeRelative( NewPath, GetGameDirectory() );
                    auto MapNode = mPropertyTemplates.extract(Iter);
                    MapNode.key() = kNewTypeName;
                    MapNode.mapped().Path = RelativePath;
                    mPropertyTemplates.insert( std::move(MapNode) );
                    mDirty = true;

                    // Renaming the archetype will handle updating the actual type name, and
                    // dirtying/invalidating property sub-instances.
                    TString OldTypeName = pArchetype->HashableTypeName();
                    pArchetype->SetName(kNewTypeName);

                    // For MP2 and up, we also need to update the type names stored in the property map.
                    if (pArchetype->Game() >= EGame::EchoesDemo)
                    {
                        NPropertyMap::ChangeTypeName(pArchetype, *OldTypeName, *kNewTypeName);
                    }

                    // MP1 has a lot of unnamed properties that just use the type name as their name.
                    // Update these properties so their name now refers to the updated type name.
                    else
                    {
                        std::list<IProperty*> SubInstances;
                        pArchetype->GatherAllSubInstances(SubInstances, true);

                        for (auto Iter = SubInstances.begin(); Iter != SubInstances.end(); Iter++)
                        {
                            IProperty* pProperty = *Iter;

                            if (pProperty->Name() == kTypeName)
                            {
                                pProperty->SetName(kNewTypeName);
                            }
                        }
                    }

                    return true;
                }
            }
        }
    }

    return false;
}

CScriptTemplate* CGameTemplate::FindMiscTemplate(const TString& kTemplateName)
{
    auto Iter = mMiscTemplates.find(kTemplateName);

    if (Iter == mMiscTemplates.end())
    {
        return nullptr;
    }
    else
    {
        SScriptTemplatePath& Path = Iter->second;
        return Path.pTemplate.get();
    }
}

TString CGameTemplate::GetGameDirectory() const
{
    return mSourceFile.GetFileDirectory();
}
