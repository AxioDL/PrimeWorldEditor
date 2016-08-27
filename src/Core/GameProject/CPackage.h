#ifndef CPACKAGE
#define CPACKAGE

#include <Common/CAssetID.h>
#include <Common/CFourCC.h>
#include <Common/TString.h>
#include <Common/Serialization/IArchive.h>

class CGameProject;

struct SNamedResource
{
    TString Name;
    CAssetID ID;
    CFourCC Type;

    void Serialize(IArchive& rArc)
    {
        rArc << SERIAL_AUTO(Name) << SERIAL_AUTO(ID) << SERIAL_AUTO(Type);
    }
};

class CResourceCollection
{
    TString mName;
    std::vector<SNamedResource> mNamedResources;

public:
    CResourceCollection() : mName("UNNAMED") {}
    CResourceCollection(const TString& rkName) : mName(rkName) {}

    void Serialize(IArchive& rArc)
    {
        rArc << SERIAL("Name", mName) << SERIAL_CONTAINER("Resources", mNamedResources, "Resource");
    }

    inline TString Name() const                                 { return mName; }
    inline u32 NumResources() const                             { return mNamedResources.size(); }
    inline const SNamedResource& ResourceByIndex(u32 Idx) const { return mNamedResources[Idx]; }

    inline void AddResource(const TString& rkName, const CAssetID& rkID, const CFourCC& rkType)
    {
        mNamedResources.push_back( SNamedResource { rkName, rkID, rkType } );
    }
};

class CPackage
{
    CGameProject *mpProject;
    TString mPakName;
    TWideString mPakPath;
    std::vector<CResourceCollection*> mCollections;

    enum EPackageDefinitionVersion
    {
        eVer_Initial,

        eVer_Max,
        eVer_Current = eVer_Max - 1
    };

public:
    CPackage() {}

    CPackage(CGameProject *pProj, const TString& rkName, const TWideString& rkPath)
        : mpProject(pProj)
        , mPakName(rkName)
        , mPakPath(rkPath)
    {}

    void Load();
    void Save();
    void Serialize(IArchive& rArc);

    void Cook();
    void CompareOriginalAssetList(const std::list<CAssetID>& rkNewList);

    TWideString DefinitionPath(bool Relative) const;
    TWideString CookedPackagePath(bool Relative) const;

    CResourceCollection* AddCollection(const TString& rkName);
    void RemoveCollection(CResourceCollection *pCollection);
    void RemoveCollection(u32 Index);

    // Accessors
    inline TString Name() const                                     { return mPakName; }
    inline TWideString Path() const                                 { return mPakPath; }
    inline CGameProject* Project() const                            { return mpProject; }
    inline u32 NumCollections() const                               { return mCollections.size(); }
    inline CResourceCollection* CollectionByIndex(u32 Idx) const    { return mCollections[Idx]; }

    inline void SetPakName(TString NewName) { mPakName = NewName; }
};

#endif // CPACKAGE

