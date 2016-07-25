#ifndef CDEPENDENCYTREE
#define CDEPENDENCYTREE

#include "CResourceEntry.h"
#include <FileIO/FileIO.h>
#include <Common/AssertMacro.h>
#include <Common/CAssetID.h>

class CScriptLayer;
class CScriptObject;
class CPropertyStruct;
class TCharacterProperty;

// Group of node classes forming a tree of cached resource dependencies.
enum EDependencyNodeType
{
    eDNT_Root,
    eDNT_AnimSet,
    eDNT_ScriptInstance,
    eDNT_Area,
    eDNT_ResourceDependency,
    eDNT_AnimSetDependency
};

// Base class providing an interface for reading/writing to cache file and determining type.
class IDependencyNode
{
public:
    virtual ~IDependencyNode() {}
    virtual EDependencyNodeType Type() const = 0;
    virtual void Read(IInputStream& rFile, EIDLength IDLength) = 0;
    virtual void Write(IOutputStream& rFile, EIDLength IDLength) const = 0;
};

// Node representing a single resource dependency.
class CResourceDependency : public IDependencyNode
{
    CAssetID mID;

public:
    CResourceDependency() {}
    CResourceDependency(const CAssetID& rkID) : mID(rkID) {}

    virtual EDependencyNodeType Type() const;
    virtual void Read(IInputStream& rFile, EIDLength IDLength);
    virtual void Write(IOutputStream& rFile, EIDLength IDLength) const;

    // Accessors
    inline CAssetID ID() const              { return mID; }
    inline void SetID(const CAssetID& rkID) { mID = rkID; }
};

// Node representing a single animset dependency contained in a script object. Indicates which character is being used.
class CAnimSetDependency : public CResourceDependency
{
protected:
    u32 mUsedChar;

public:
    CAnimSetDependency() : CResourceDependency(), mUsedChar(-1) {}

    virtual EDependencyNodeType Type() const;
    virtual void Read(IInputStream& rFile, EIDLength IDLength);
    virtual void Write(IOutputStream& rFile, EIDLength IDLength) const;

    // Accessors
    inline u32 UsedChar() const                 { return mUsedChar; }
    inline void SetUsedChar(u32 CharIdx)        { mUsedChar = CharIdx; }

    // Static
    static CAnimSetDependency* BuildDependency(TCharacterProperty *pProp);
};

// Tree root node, representing a resource.
class CDependencyTree : public IDependencyNode
{
protected:
    CAssetID mID;
    std::vector<CResourceDependency*> mReferencedResources;

public:
    CDependencyTree(const CAssetID& rkID) : mID(rkID) {}
    ~CDependencyTree();

    virtual EDependencyNodeType Type() const;
    virtual void Read(IInputStream& rFile, EIDLength IDLength);
    virtual void Write(IOutputStream& rFile, EIDLength IDLength) const;

    u32 NumDependencies() const;
    bool HasDependency(const CAssetID& rkID);
    CAssetID DependencyByIndex(u32 Index) const;
    void AddDependency(const CAssetID& rkID);
    void AddDependency(CResource *pRes);

    // Accessors
    inline void SetID(const CAssetID& rkID) { mID = rkID; }
    inline CAssetID ID() const              { return mID; }

};

// Node representing an animset resource; allows for lookup of dependencies of a particular character in the set.
class CAnimSetDependencyTree : public CDependencyTree
{
protected:
    std::vector<u32> mCharacterOffsets;

public:
    CAnimSetDependencyTree(const CAssetID& rkID) : CDependencyTree(rkID) {}
    virtual EDependencyNodeType Type() const;
    virtual void Read(IInputStream& rFile, EIDLength IDLength);
    virtual void Write(IOutputStream& rFile, EIDLength IDLength) const;
};

// Node representing a script object. Indicates the type of object.
class CScriptInstanceDependencyTree : public IDependencyNode
{
protected:
    u32 mObjectType;
    std::vector<CResourceDependency*> mDependencies;

public:
    ~CScriptInstanceDependencyTree();

    virtual EDependencyNodeType Type() const;
    virtual void Read(IInputStream& rFile, EIDLength IDLength);
    virtual void Write(IOutputStream& rFile, EIDLength IDLength) const;
    bool HasDependency(const CAssetID& rkID);

    // Accessors
    u32 NumDependencies() const { return mDependencies.size(); }

    // Static
    static CScriptInstanceDependencyTree* BuildTree(CScriptObject *pInstance);
    static void ParseStructDependencies(CScriptInstanceDependencyTree *pTree, CPropertyStruct *pStruct);
};

// Node representing an area. Tracks dependencies on a per-instance basis and can separate dependencies of different script layers.
class CAreaDependencyTree : public CDependencyTree
{
protected:
    std::vector<CScriptInstanceDependencyTree*> mScriptInstances;
    std::vector<u32> mLayerOffsets;

public:
    CAreaDependencyTree(const CAssetID& rkID) : CDependencyTree(rkID) {}
    ~CAreaDependencyTree();

    virtual EDependencyNodeType Type() const;
    virtual void Read(IInputStream& rFile, EIDLength IDLength);
    virtual void Write(IOutputStream& rFile, EIDLength IDLength) const;

    void AddScriptLayer(CScriptLayer *pLayer);
};

#endif // CDEPENDENCYTREE

