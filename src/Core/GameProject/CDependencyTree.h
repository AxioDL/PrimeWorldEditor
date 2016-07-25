#ifndef CDEPENDENCYTREE
#define CDEPENDENCYTREE

#include "CResourceEntry.h"
#include <FileIO/FileIO.h>
#include <Common/AssertMacro.h>
#include <Common/CUniqueID.h>

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
    virtual void Read(IInputStream& rFile, EUIDLength IDLength) = 0;
    virtual void Write(IOutputStream& rFile, EUIDLength IDLength) const = 0;
};

// Node representing a single resource dependency.
class CResourceDependency : public IDependencyNode
{
    CUniqueID mID;

public:
    CResourceDependency() {}
    CResourceDependency(const CUniqueID& rkID) : mID(rkID) {}

    virtual EDependencyNodeType Type() const;
    virtual void Read(IInputStream& rFile, EUIDLength IDLength);
    virtual void Write(IOutputStream& rFile, EUIDLength IDLength) const;

    // Accessors
    inline CUniqueID ID() const                 { return mID; }
    inline void SetID(const CUniqueID& rkID)    { mID = rkID; }
};

// Node representing a single animset dependency contained in a script object. Indicates which character is being used.
class CAnimSetDependency : public CResourceDependency
{
protected:
    u32 mUsedChar;

public:
    CAnimSetDependency() : CResourceDependency(), mUsedChar(-1) {}

    virtual EDependencyNodeType Type() const;
    virtual void Read(IInputStream& rFile, EUIDLength IDLength);
    virtual void Write(IOutputStream& rFile, EUIDLength IDLength) const;

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
    CUniqueID mID;
    std::vector<CResourceDependency*> mReferencedResources;

public:
    CDependencyTree(const CUniqueID& rkID) : mID(rkID) {}
    ~CDependencyTree();

    virtual EDependencyNodeType Type() const;
    virtual void Read(IInputStream& rFile, EUIDLength IDLength);
    virtual void Write(IOutputStream& rFile, EUIDLength IDLength) const;

    u32 NumDependencies() const;
    bool HasDependency(const CUniqueID& rkID);
    CUniqueID DependencyByIndex(u32 Index) const;
    void AddDependency(const CUniqueID& rkID);
    void AddDependency(CResource *pRes);

    // Accessors
    inline void SetID(const CUniqueID& rkID)    { mID = rkID; }
    inline CUniqueID ID() const                 { return mID; }

};

// Node representing an animset resource; allows for lookup of dependencies of a particular character in the set.
class CAnimSetDependencyTree : public CDependencyTree
{
protected:
    std::vector<u32> mCharacterOffsets;

public:
    CAnimSetDependencyTree(const CUniqueID& rkID) : CDependencyTree(rkID) {}
    virtual EDependencyNodeType Type() const;
    virtual void Read(IInputStream& rFile, EUIDLength IDLength);
    virtual void Write(IOutputStream& rFile, EUIDLength IDLength) const;
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
    virtual void Read(IInputStream& rFile, EUIDLength IDLength);
    virtual void Write(IOutputStream& rFile, EUIDLength IDLength) const;
    bool HasDependency(const CUniqueID& rkID);

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
    CAreaDependencyTree(const CUniqueID& rkID) : CDependencyTree(rkID) {}
    ~CAreaDependencyTree();

    virtual EDependencyNodeType Type() const;
    virtual void Read(IInputStream& rFile, EUIDLength IDLength);
    virtual void Write(IOutputStream& rFile, EUIDLength IDLength) const;

    void AddScriptLayer(CScriptLayer *pLayer);
};

#endif // CDEPENDENCYTREE

