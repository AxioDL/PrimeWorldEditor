#ifndef CDEPENDENCYTREE
#define CDEPENDENCYTREE

#include "CResourceEntry.h"
#include <Common/AssertMacro.h>
#include <Common/CAssetID.h>
#include <Common/FileIO.h>

class CScriptLayer;
class CScriptObject;
class CStructProperty;
class CAnimSet;
class CAnimationParameters;
struct SSetCharacter;

// Group of node classes forming a tree of cached resource dependencies.
enum EDependencyNodeType
{
    eDNT_DependencyTree     = FOURCC('TREE'),
    eDNT_ResourceDependency = FOURCC('RSDP'),
    eDNT_ScriptInstance     = FOURCC('SCIN'),
    eDNT_ScriptProperty     = FOURCC('SCPR'),
    eDNT_CharacterProperty  = FOURCC('CRPR'),
    eDNT_SetCharacter       = FOURCC('SCHR'),
    eDNT_SetAnimation       = FOURCC('SANM'),
    eDNT_AnimEvent          = FOURCC('EVNT'),
    eDNT_Area               = FOURCC('AREA'),
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
    virtual void GetAllResourceReferences(std::set<CAssetID>& rOutSet) const;
    virtual bool HasDependency(const CAssetID& rkID) const;

    // Serialization constructor
    static IDependencyNode* ArchiveConstructor(EDependencyNodeType Type);

    // Accessors
    inline u32 NumChildren() const                          { return mChildren.size(); }
    inline IDependencyNode* ChildByIndex(u32 Index) const   { return mChildren[Index]; }
};

// Basic dependency tree; this class is sufficient for most resource types.
class CDependencyTree : public IDependencyNode
{
public:
    CDependencyTree() {}

    virtual EDependencyNodeType Type() const;
    virtual void Serialize(IArchive& rArc);

    void AddChild(IDependencyNode *pNode);
    void AddDependency(const CAssetID& rkID, bool AvoidDuplicates = true);
    void AddDependency(CResource *pRes, bool AvoidDuplicates = true);
    void AddCharacterDependency(const CAnimationParameters& rkAnimParams);
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
    virtual void GetAllResourceReferences(std::set<CAssetID>& rOutSet) const;
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
    static void ParseStructDependencies(CScriptInstanceDependency *pTree, CScriptObject* pInstance, CStructProperty *pStruct);
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
    static CSetCharacterDependency* BuildTree(const SSetCharacter& rkChar);
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
    inline bool IsUsedByAnyCharacter() const            { return !mCharacterIndices.empty(); }

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

    virtual EDependencyNodeType Type() const;
    virtual void Serialize(IArchive& rArc);

    void AddScriptLayer(CScriptLayer *pLayer, const std::vector<CAssetID>& rkExtraDeps);
    void GetModuleDependencies(EGame Game, std::vector<TString>& rModuleDepsOut, std::vector<u32>& rModuleLayerOffsetsOut) const;

    // Accessors
    inline u32 NumScriptLayers() const                  { return mLayerOffsets.size(); }
    inline u32 ScriptLayerOffset(u32 LayerIdx) const    { return mLayerOffsets[LayerIdx]; }
};

#endif // CDEPENDENCYTREE

