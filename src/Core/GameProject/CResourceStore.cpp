#include "CResourceStore.h"
#include "CGameExporter.h"
#include "CGameProject.h"
#include "Core/Resource/CResource.h"
#include <Common/AssertMacro.h>
#include <Common/FileUtil.h>
#include <Common/Log.h>
#include <tinyxml2.h>

using namespace tinyxml2;
CResourceStore gResourceStore;

CResourceStore::CResourceStore()
    : mpProj(nullptr)
    , mpProjectRoot(nullptr)
    , mpExporter(nullptr)
{}

CResourceStore::~CResourceStore()
{
}

void CResourceStore::LoadResourceDatabase(const TString& rkPath)
{
    XMLDocument Doc;
    Doc.LoadFile(*rkPath);

    if (!Doc.Error())
    {
        XMLElement *pRoot = Doc.FirstChildElement("ResourceDatabase");
        //EDatabaseVersion Version = (EDatabaseVersion) TString(pRoot->Attribute("Version")).ToInt32(10); // Version currently unused

        XMLElement *pResources = pRoot->FirstChildElement("Resources");
        XMLElement *pRes = pResources->FirstChildElement("Resource");
        u32 ResIndex = 0;

        while (pRes)
        {
            XMLElement *pChild = pRes->FirstChildElement();

            bool HasID = false, HasType = false, HasDir = false, HasName = false;
            CUniqueID ID;
            EResType Type;
            TWideString FileDir;
            TWideString FileName;

            while (pChild)
            {
                TString NodeName = pChild->Name();

                if (NodeName == "ID")
                {
                    ID = CUniqueID::FromString(pChild->GetText());
                    HasID = true;
                }

                else if (NodeName == "Type")
                {
                    Type = CResource::ResTypeForExtension(pChild->GetText());
                    HasType = true;
                    ASSERT(Type != eInvalidResType);
                }

                else if (NodeName == "FileDir")
                {
                    FileDir = pChild->GetText();
                    HasDir = true;
                }

                else if (NodeName == "FileName")
                {
                    FileName = pChild->GetText();
                    HasName = true;
                }

                pChild = pChild->NextSiblingElement();
            }

            if (HasID && HasType && HasDir && HasName)
                RegisterResource(ID, Type, FileDir, FileName);
            else
                Log::Error("Error reading " + rkPath + ": Resource entry " + TString::FromInt32(ResIndex, 0, 10) + " is missing one or more components");

            ResIndex++;
            pRes = pRes->NextSiblingElement("Resource");
        }
    }
}

void CResourceStore::SaveResourceDatabase(const TString& rkPath) const
{
    XMLDocument Doc;

    XMLDeclaration *pDecl = Doc.NewDeclaration();
    Doc.LinkEndChild(pDecl);

    XMLElement *pRoot = Doc.NewElement("ResourceDatabase");
    pRoot->SetAttribute("Version", eVer_Current);
    Doc.LinkEndChild(pRoot);

    XMLElement *pResources = Doc.NewElement("Resources");
    pRoot->LinkEndChild(pResources);

    for (auto It = mResourceEntries.begin(); It != mResourceEntries.end(); It++)
    {
        CResourceEntry *pEntry = It->second;
        if (pEntry->IsTransient()) continue;

        XMLElement *pRes = Doc.NewElement("Resource");
        pResources->LinkEndChild(pRes);

        XMLElement *pID = Doc.NewElement("ID");
        pID->SetText(*pEntry->ID().ToString());
        pRes->LinkEndChild(pID);

        XMLElement *pType = Doc.NewElement("Type");
        pType->SetText(*GetResourceCookedExtension(pEntry->ResourceType(), pEntry->Game()));
        pRes->LinkEndChild(pType);

        XMLElement *pDir = Doc.NewElement("FileDir");
        pDir->SetText(*pEntry->Directory()->FullPath().ToUTF8());
        pRes->LinkEndChild(pDir);

        XMLElement *pName = Doc.NewElement("FileName");
        pName->SetText(*pEntry->FileName());
        pRes->LinkEndChild(pName);

        XMLElement *pRecook = Doc.NewElement("NeedsRecook");
        pRecook->SetText(pEntry->NeedsRecook() ? "true" : "false");
        pRes->LinkEndChild(pRecook);
    }

    Doc.SaveFile(*rkPath);
}

void CResourceStore::SetActiveProject(CGameProject *pProj)
{
    if (mpProj == pProj) return;

    CloseActiveProject();
    mpProj = pProj;

    if (pProj)
    {
        mpProjectRoot = new CVirtualDirectory();
        LoadResourceDatabase(pProj->ResourceDBPath(false));
    }
}

void CResourceStore::CloseActiveProject()
{
    // Delete all entries from old project
    for (auto It = mResourceEntries.begin(); It != mResourceEntries.end(); It++)
    {
        CResourceEntry *pEntry = It->second;
        if (!pEntry->IsTransient())
        {
            delete pEntry;
            It = mResourceEntries.erase(It);
        }
    }

    delete mpProjectRoot;
    mpProjectRoot = nullptr;
    mpProj = nullptr;
}

CVirtualDirectory* CResourceStore::GetVirtualDirectory(const TWideString& rkPath, bool Transient, bool AllowCreate)
{
    if (rkPath.IsEmpty()) return nullptr;

    else if (Transient)
    {
        for (u32 iTrans = 0; iTrans < mTransientRoots.size(); iTrans++)
        {
            if (mTransientRoots[iTrans]->Name() == rkPath)
                return mTransientRoots[iTrans];
        }

        if (AllowCreate)
        {
            CVirtualDirectory *pDir = new CVirtualDirectory(rkPath);
            mTransientRoots.push_back(pDir);
            return pDir;
        }

        else return nullptr;
    }

    else if (mpProjectRoot)
    {
        return mpProjectRoot->FindChildDirectory(rkPath, AllowCreate);
    }

    else return nullptr;
}

CResourceEntry* CResourceStore::FindEntry(const CUniqueID& rkID) const
{
    if (!rkID.IsValid()) return nullptr;
    auto Found = mResourceEntries.find(rkID);
    if (Found == mResourceEntries.end()) return nullptr;
    else return Found->second;
}

bool CResourceStore::RegisterResource(const CUniqueID& rkID, EResType Type, const TWideString& rkDir, const TWideString& rkFileName)
{
    CResourceEntry *pEntry = FindEntry(rkID);

    if (pEntry)
    {
        Log::Error("Attempted to register resource that's already tracked in the database: " + rkID.ToString() + " / " + rkDir.ToUTF8() + " / " + rkFileName.ToUTF8());
        return false;
    }

    else
    {
        pEntry = new CResourceEntry(this, rkID, rkDir, rkFileName.GetFileName(false), Type);

        if (!pEntry->HasCookedVersion() && !pEntry->HasRawVersion())
        {
            Log::Error("Attempted to register a resource that doesn't exist: " + rkID.ToString() + " | " + rkDir.ToUTF8() + " | " + rkFileName.ToUTF8());
            delete pEntry;
            return false;
        }

        else
        {
            mResourceEntries[rkID] = pEntry;
            return true;
        }
    }
}

CResourceEntry* CResourceStore::RegisterTransientResource(EResType Type, const TWideString& rkDir /*= L""*/, const TWideString& rkFileName /*= L""*/)
{
    CResourceEntry *pEntry = new CResourceEntry(this, CUniqueID::RandomID(), rkDir, rkFileName, Type, true);
    mResourceEntries[pEntry->ID()] = pEntry;
    return pEntry;
}

CResourceEntry* CResourceStore::RegisterTransientResource(EResType Type, const CUniqueID& rkID, const TWideString& rkDir /*=L ""*/, const TWideString& rkFileName /*= L""*/)
{
    CResourceEntry *pEntry = new CResourceEntry(this, rkID, rkDir, rkFileName, Type, true);
    mResourceEntries[rkID] = pEntry;
    return pEntry;
}

CResource* CResourceStore::LoadResource(const CUniqueID& rkID, const CFourCC& rkType)
{
    if (!rkID.IsValid()) return nullptr;

    // Check if resource is already loaded
    auto Find = mLoadedResources.find(rkID);
    if (Find != mLoadedResources.end())
        return Find->second->Resource();

    // With Game Exporter - Get data buffer from exporter
    if (mpExporter)
    {
        std::vector<u8> DataBuffer;
        mpExporter->LoadResource(rkID, DataBuffer);
        if (DataBuffer.empty()) return nullptr;

        CMemoryInStream MemStream(DataBuffer.data(), DataBuffer.size(), IOUtil::eBigEndian);
        EResType Type = CResource::ResTypeForExtension(rkType);
        CResourceEntry *pEntry = RegisterTransientResource(Type, rkID);
        CResource *pRes = pEntry->Load(MemStream);
        if (pRes) mLoadedResources[rkID] = pEntry;
        return pRes;
    }

    // Without Game Exporter - Check store resource entries and transient load directory.
    else
    {
        // Check for resource in store
        CResourceEntry *pEntry = FindEntry(rkID);
        if (pEntry) return pEntry->Load();

        // Check in transient load directory
        EResType Type = CResource::ResTypeForExtension(rkType);

        if (Type != eInvalidResType)
        {
            // Note the entry may not be able to find the resource on its own (due to not knowing what game
            // it is) so we will attempt to open the file stream ourselves and pass it to the entry instead.
            TString Name = rkID.ToString();
            CResourceEntry *pEntry = RegisterTransientResource(Type, mTransientLoadDir, Name.ToUTF16());

            TString Path = mTransientLoadDir.ToUTF8() + Name + "." + rkType.ToString();
            CFileInStream File(Path.ToStdString(), IOUtil::eBigEndian);
            CResource *pRes = pEntry->Load(File);

            if (pRes) mLoadedResources[rkID] = pEntry;
            return pRes;
        }

        else
        {
            Log::Error("Can't load requested resource with ID \"" + rkID.ToString() + "\"; can't locate resource. Note: Loading raw assets from an arbitrary directory is unsupported.");;
            return nullptr;
        }
    }
}

CResource* CResourceStore::LoadResource(const TString& rkPath)
{
    // Construct ID from string, check if resource is loaded already
    TWideString Dir = FileUtil::MakeAbsolute(TWideString(rkPath.GetFileDirectory()));
    TString Name = rkPath.GetFileName(false);
    CUniqueID ID = (Name.IsHexString() ? Name.ToInt64() : rkPath.Hash64());
    auto Find = mLoadedResources.find(ID);

    if (Find != mLoadedResources.end())
        return Find->second->Resource();

    // Determine type
    TString Extension = rkPath.GetFileExtension().ToUpper();
    EResType Type = CResource::ResTypeForExtension(Extension);

    if (Type == eInvalidResType)
    {
        Log::Error("Unable to load resource " + rkPath + "; unrecognized extension: " + Extension);
        return nullptr;
    }

    // Open file
    CFileInStream File(rkPath.ToStdString(), IOUtil::eBigEndian);

    if (!File.IsValid())
    {
        Log::Error("Unable to load resource; couldn't open file: " + rkPath);
        return nullptr;
    }

    // Load resource
    TString OldTransientDir = mTransientLoadDir;
    mTransientLoadDir = Dir;

    CResourceEntry *pEntry = RegisterTransientResource(Type, ID, Dir, Name);
    CResource *pRes = pEntry->Load(File);

    if (pRes) mLoadedResources[ID] = pEntry;
    mTransientLoadDir = OldTransientDir;

    return pRes;
}

CFourCC CResourceStore::ResourceTypeByID(const CUniqueID& rkID, const TStringList& rkPossibleTypes) const
{
    if (!rkID.IsValid()) return eInvalidResType;
    if (rkPossibleTypes.size() == 1) return CFourCC(rkPossibleTypes.front());

    // Check for existing entry
    auto Find = mResourceEntries.find(rkID);
    if (Find != mResourceEntries.end())
        return GetResourceCookedExtension(Find->second->ResourceType(), Find->second->Game());

    // Determine extension from filesystem - try every extension until we find the file
    TString PathBase = mTransientLoadDir.ToUTF8() + rkID.ToString() + '.';

    for (auto It = rkPossibleTypes.begin(); It != rkPossibleTypes.end(); It++)
    {
        TString NewPath = PathBase + *It;

        if (FileUtil::Exists(NewPath))
            return CFourCC(*It);
    }

    // Couldn't find one, so return unknown. Note that it'd be possible to look up the extension from the
    // filesystem even if it's not one of the provided possible types, but this would be too slow.
    return "UNKN";
}

void CResourceStore::DestroyUnreferencedResources()
{
    // This can be updated to avoid the do-while loop when reference lookup is implemented.
    u32 NumDeleted;

    do
    {
        NumDeleted = 0;

        for (auto It = mLoadedResources.begin(); It != mLoadedResources.end(); It++)
        {
            CResourceEntry *pEntry = It->second;

            if (!pEntry->Resource()->IsReferenced())
            {
                bool Unloaded = pEntry->Unload();

                if (Unloaded)
                {
                    It = mLoadedResources.erase(It);
                    NumDeleted++;
                    if (It == mLoadedResources.end()) break;
                }
            }
        }
    } while (NumDeleted > 0);

    // Destroy empty virtual directories
    for (auto DirIt = mTransientRoots.begin(); DirIt != mTransientRoots.end(); DirIt++)
    {
        CVirtualDirectory *pRoot = *DirIt;

        if (pRoot->IsEmpty())
        {
            delete pRoot;
            DirIt = mTransientRoots.erase(DirIt);
        }
    }

    Log::Write(TString::FromInt32(mLoadedResources.size(), 0, 10) + " resources loaded");
}

void CResourceStore::SetTransientLoadDir(const TString& rkDir)
{
    mTransientLoadDir = rkDir;
    mTransientLoadDir.EnsureEndsWith('\\');
    Log::Write("Set resource directory: " + rkDir);
}
