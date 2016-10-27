#ifndef CDEPENDENCYTREE
#define CDEPENDENCYTREE

#include "CResourceEntry.h"
#include <FileIO/FileIO.h>
#include <Common/AssertMacro.h>
#include <Common/CAssetID.h>

class CScriptLayer;
class CScriptObject;
class CPropertyStruct;
class CAnimSet;
struct SSetCharacter;

// Group of node classes forming a tree of cached resource dependencies.
enum EDependencyNodeType
{
    eDNT_DependencyTree     = FOURCC_CONSTEXPR('T', 'R', 'E', 'E'),
    eDNT_ResourceDependency = FOURCC_CONSTEXPR('R', 'S', 'D', 'P'),
    eDNT_ScriptInstance     = FOURCC_CONSTEXPR('S', 'C', 'I', 'N'),
    eDNT_ScriptProperty     = FOURCC_CONSTEXPR('S', 'C', 'P', 'R'),
    eDNT_CharacterProperty  = FOURCC_CONSTEXPR('C', 'R', 'P', 'R'),
    eDNT_SetCharacter       = FOURCC_CONSTEXPR('S', 'C', 'H', 'R'),
    eDNT_SetAnimation       = FOURCC_CONSTEXPR('S', 'A', 'N', 'M'),
    eDNT_AnimEvent          = FOURCC_CONSTEXPR('E', 'V', 'N', 'T'),
    eDNT_Area               = FOURCC_CONSTEXPR('A', 'R', 'E', 'A'),
};

// Base class providing an interface for a basic dependency node.
class IDependencyNode
{
protected:
    std::vector<IDependencyNode*> mChildren;

public:
    virtual ~IDependencyNode();
    virtual EDependencyNodeType Type() const = 0;
    virtual void Serialize(IArchive& rArc) = 0;
    virtual bool HasDependency(const CAssetID& rkID) const;

    // Accessors
    u32 NumChildren() const                         { return mChildren.size(); }
    IDependencyNode* ChildByIndex(u32 Index) const  { return mChildren[Index]; }
};

// Basic dependency tree; this class is sufficient for most resource types.
class CDependencyTree : public IDependencyNode
{
protected:
    CAssetID mRootID;

public:
    CDependencyTree() {}
    CDependencyTree(const CAssetID& rkID) : mRootID(rkID) {}

    virtual EDependencyNodeType Type() const;
    virtual void Serialize(IArchive& rArc);

    void AddChild(IDependencyNode *pNode);
    void AddDependency(const CAssetID& rkID, bool AvoidDuplicates = true);
    void AddDependency(CResource *pRes, bool AvoidDuplicates = true);

    // Accessors
    inline void SetID(const CAssetID& rkID) { mRootID = rkID; }
    inline CAssetID ID() const              { return mRootID; }
};

// Node representing a single resource dependency.
class CResourceDependency : public IDependencyNode
{
protected:
    CAssetID mID;

public:
    CResourceDependency() {}
    CResourceDependency(const CAssetID& rkID) : mID(rkID) {}

    virtual EDependencyNodeType Type() const;
    virtual void Serialize(IArchive& rArc);
    virtual bool HasDependency(const CAssetID& rkID) const;

    // Accessors
    inline CAssetID ID() const              { return mID; }
    inline void SetID(const CAssetID& rkID) { mID = rkID; }
};

// Node representing a single resource dependency referenced by a script property.
class CPropertyDependency : public CResourceDependency
{
    TString mIDString;

public:
    CPropertyDependency()
        : CResourceDependency()
    {}

    CPropertyDependency(const TString& rkPropID, const CAssetID& rkAssetID)
        : CResourceDependency(rkAssetID)
        , mIDString(rkPropID)
    {}

    virtual EDependencyNodeType Type() const;
    virtual void Serialize(IArchive& rArc);

    // Accessors
    inline TString PropertyID() const   { return mIDString; }
};

// Node representing a single animset dependency referenced by a script property. Indicates which character is being used.
class CCharPropertyDependency : public CPropertyDependency
{
protected:
    u32 mUsedChar;

public:
    CCharPropertyDependency()
        : CPropertyDependency()
        , mUsedChar(-1)
    {}

    CCharPropertyDependency(const TString& rkPropID, const CAssetID& rkAssetID, u32 UsedChar)
        : CPropertyDependency(rkPropID, rkAssetID)
        , mUsedChar(UsedChar)
    {}

    virtual EDependencyNodeType Type() const;
    virtual void Serialize(IArchive& rArc);

    // Accessors
    inline u32 UsedChar() const                 { return mUsedChar; }
};

// Node representing a script object. Indicates the type of object.
class CScriptInstanceDependency : public IDependencyNode
{
protected:
    u32 mObjectType;

public:
    virtual EDependencyNodeType Type() const;
    virtual void Serialize(IArchive& rArc);

    // Accessors
    inline u32 ObjectType() const       { return mObjectType; }

    // Static
    static CScriptInstanceDependency* BuildTree(CScriptObject *pInstance);
protected:
    static void ParseStructDependencies(CScriptInstanceDependency *pTree, CPropertyStruct *pStruct);
};

// Node representing an animset character. Indicates what index the character is within the animset.
class CSetCharacterDependency : public CDependencyTree
{
protected:
    u32 mCharSetIndex;

public:
    CSetCharacterDependency() : CDependencyTree() {}
    CSetCharacterDependency(u32 SetIndex) : CDependencyTree(), mCharSetIndex(SetIndex) {}

    virtual EDependencyNodeType Type() const;
    virtual void Serialize(IArchive& rArc);

    // Accessors
    inline u32 CharSetIndex() const { return mCharSetIndex; }

    // Static
    static CSetCharacterDependency* BuildTree(const CAnimSet *pkOwnerSet, u32 CharIndex);
};

// Node representing a character animation. Indicates which character indices use this animation.
class CSetAnimationDependency : public CDependencyTree
{
protected:
    std::set<u32> mCharacterIndices;

public:
    CSetAnimationDependency() : CDependencyTree() {}

    virtual EDependencyNodeType Type() const;
    virtual void Serialize(IArchive& rArc);

    // Accessors
    inline bool IsUsedByCharacter(u32 CharIdx) const    { return mCharacterIndices.find(CharIdx) != mCharacterIndices.end(); }

    // Static
    static CSetAnimationDependency* BuildTree(const CAnimSet *pkOwnerSet, u32 AnimIndex);
};

// Node representing an animation event. Indicates which character index uses this event.
class CAnimEventDependency : public CResourceDependency
{
protected:
    u32 mCharIndex;

public:
    CAnimEventDependency() : CResourceDependency() {}
    CAnimEventDependency(const CAssetID& rkID, u32 CharIndex)
        : CResourceDependency(rkID), mCharIndex(CharIndex) {}

    virtual EDependencyNodeType Type() const;
    virtual void Serialize(IArchive& rArc);

    // Accessors
    inline u32 CharIndex() const    { return mCharIndex; }
};

// Node representing an area. Tracks dependencies on a per-instance basis and can separate dependencies of different script layers.
class CAreaDependencyTree : public CDependencyTree
{
protected:
    std::vector<u32> mLayerOffsets;

public:
    CAreaDependencyTree() : CDependencyTree() {}
    CAreaDependencyTree(const CAssetID& rkID) : CDependencyTree(rkID) {}

    virtual EDependencyNodeType Type() const;
    virtual void Serialize(IArchive& rArc);

    void AddScriptLayer(CScriptLayer *pLayer);
    void GetModuleDependencies(EGame Game, std::vector<TString>& rModuleDepsOut, std::vector<u32>& rModuleLayerOffsetsOut) const;

    // Accessors
    inline u32 NumScriptLayers() const                  { return mLayerOffsets.size(); }
    inline u32 ScriptLayerOffset(u32 LayerIdx) const    { return mLayerOffsets[LayerIdx]; }
};

// Dependency node factory for serialization
class CDependencyNodeFactory
{
public:
    IDependencyNode* SpawnObject(u32 NodeID)
    {
        switch (NodeID)
        {
        case eDNT_DependencyTree:       return new CDependencyTree;
        case eDNT_ResourceDependency:   return new CResourceDependency;
        case eDNT_ScriptInstance:       return new CScriptInstanceDependency;
        case eDNT_ScriptProperty:       return new CPropertyDependency;
        case eDNT_CharacterProperty:    return new CCharPropertyDependency;
        case eDNT_SetCharacter:         return new CSetCharacterDependency;
        case eDNT_SetAnimation:         return new CSetAnimationDependency;
        case eDNT_AnimEvent:            return new CAnimEventDependency;
        case eDNT_Area:                 return new CAreaDependencyTree;
        default:                        ASSERT(false); return nullptr;
        }
    }
};
extern CDependencyNodeFactory gDependencyNodeFactory;

#endif // CDEPENDENCYTREE

