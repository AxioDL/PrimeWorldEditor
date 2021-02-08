#ifndef CDEPENDENCYTREE
#define CDEPENDENCYTREE

#include "CResourceEntry.h"
#include <Common/CAssetID.h>
#include <Common/FileIO.h>
#include <Common/Macros.h>
#include <memory>

class CScriptLayer;
class CScriptObject;
class CStructProperty;
class CAnimSet;
class CAnimationParameters;
struct SSetCharacter;

// Group of node classes forming a tree of cached resource dependencies.
enum class EDependencyNodeType
{
    DependencyTree      = FOURCC('TREE'),
    Resource            = FOURCC('RSDP'),
    ScriptInstance      = FOURCC('SCIN'),
    ScriptProperty      = FOURCC('SCPR'),
    CharacterProperty   = FOURCC('CRPR'),
    SetCharacter        = FOURCC('SCHR'),
    SetAnimation        = FOURCC('SANM'),
    AnimEvent           = FOURCC('EVNT'),
    Area                = FOURCC('AREA'),
};

// Base class providing an interface for a basic dependency node.
class IDependencyNode
{
protected:
    std::vector<std::unique_ptr<IDependencyNode>> mChildren;

public:
    virtual ~IDependencyNode();
    virtual EDependencyNodeType Type() const = 0;
    virtual void Serialize(IArchive& rArc) = 0;
    virtual void GetAllResourceReferences(std::set<CAssetID>& rOutSet) const;
    virtual bool HasDependency(const CAssetID& id) const;
    void ParseProperties(CResourceEntry* pParentEntry, CStructProperty* pProperties, void* pData);

    // Serialization constructor
    static IDependencyNode* ArchiveConstructor(EDependencyNodeType Type);

    // Accessors
    size_t NumChildren() const                         { return mChildren.size(); }
    IDependencyNode* ChildByIndex(size_t Index) const  { return mChildren[Index].get(); }
};

// Basic dependency tree; this class is sufficient for most resource types.
class CDependencyTree : public IDependencyNode
{
public:
    CDependencyTree() = default;

    EDependencyNodeType Type() const override;
    void Serialize(IArchive& rArc) override;

    void AddChild(std::unique_ptr<IDependencyNode>&& pNode);
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
    CResourceDependency() = default;
    explicit CResourceDependency(const CAssetID& rkID) : mID(rkID) {}

    EDependencyNodeType Type() const override;
    void Serialize(IArchive& rArc) override;
    void GetAllResourceReferences(std::set<CAssetID>& rOutSet) const override;
    bool HasDependency(const CAssetID& rkID) const override;

    // Accessors
    CAssetID ID() const              { return mID; }
    void SetID(const CAssetID& rkID) { mID = rkID; }
};

// Node representing a single resource dependency referenced by a script property.
class CPropertyDependency : public CResourceDependency
{
    TString mIDString;

public:
    CPropertyDependency() = default;

    CPropertyDependency(TString rkPropID, const CAssetID& rkAssetID)
        : CResourceDependency(rkAssetID)
        , mIDString(std::move(rkPropID))
    {}

    EDependencyNodeType Type() const override;
    void Serialize(IArchive& rArc) override;

    // Accessors
    TString PropertyID() const   { return mIDString; }
};

// Node representing a single animset dependency referenced by a script property. Indicates which character is being used.
class CCharPropertyDependency : public CPropertyDependency
{
protected:
    int mUsedChar = -1;

public:
    CCharPropertyDependency() = default;

    CCharPropertyDependency(TString rkPropID, const CAssetID& rkAssetID, int UsedChar)
        : CPropertyDependency(std::move(rkPropID), rkAssetID)
        , mUsedChar(UsedChar)
    {}

    EDependencyNodeType Type() const override;
    void Serialize(IArchive& rArc) override;

    // Accessors
    int UsedChar() const                 { return mUsedChar; }
};

// Node representing a script object. Indicates the type of object.
class CScriptInstanceDependency : public IDependencyNode
{
protected:
    uint32 mObjectType = 0;

public:
    EDependencyNodeType Type() const override;
    void Serialize(IArchive& rArc) override;

    // Accessors
    uint32 ObjectType() const       { return mObjectType; }

    // Static
    static std::unique_ptr<CScriptInstanceDependency> BuildTree(CScriptObject *pInstance);
};

// Node representing an animset character. Indicates what index the character is within the animset.
class CSetCharacterDependency : public CDependencyTree
{
protected:
    uint32 mCharSetIndex = 0;

public:
    CSetCharacterDependency() = default;
    explicit CSetCharacterDependency(uint32 SetIndex) : mCharSetIndex(SetIndex) {}

    EDependencyNodeType Type() const override;
    void Serialize(IArchive& rArc) override;

    // Accessors
    uint32 CharSetIndex() const { return mCharSetIndex; }

    // Static
    static std::unique_ptr<CSetCharacterDependency> BuildTree(const SSetCharacter& rkChar);
};

// Node representing a character animation. Indicates which character indices use this animation.
class CSetAnimationDependency : public CDependencyTree
{
protected:
    std::set<uint32> mCharacterIndices;

public:
    CSetAnimationDependency() = default;

    EDependencyNodeType Type() const override;
    void Serialize(IArchive& rArc) override;

    // Accessors
    bool IsUsedByCharacter(uint32 CharIdx) const { return mCharacterIndices.find(CharIdx) != mCharacterIndices.end(); }
    bool IsUsedByAnyCharacter() const            { return !mCharacterIndices.empty(); }

    // Static
    static std::unique_ptr<CSetAnimationDependency> BuildTree(const CAnimSet *pkOwnerSet, uint32 AnimIndex);
};

// Node representing an animation event. Indicates which character index uses this event.
class CAnimEventDependency : public CResourceDependency
{
protected:
    uint32 mCharIndex = 0;

public:
    CAnimEventDependency() = default;
    CAnimEventDependency(const CAssetID& rkID, uint32 CharIndex)
        : CResourceDependency(rkID), mCharIndex(CharIndex) {}

    EDependencyNodeType Type() const override;
    void Serialize(IArchive& rArc) override;

    // Accessors
    uint32 CharIndex() const    { return mCharIndex; }
};

// Node representing an area. Tracks dependencies on a per-instance basis and can separate dependencies of different script layers.
class CAreaDependencyTree : public CDependencyTree
{
protected:
    std::vector<uint32> mLayerOffsets;

public:
    CAreaDependencyTree() = default;

    EDependencyNodeType Type() const override;
    void Serialize(IArchive& rArc) override;

    void AddScriptLayer(CScriptLayer *pLayer, const std::vector<CAssetID>& rkExtraDeps);
    void GetModuleDependencies(EGame Game, std::vector<TString>& rModuleDepsOut, std::vector<uint32>& rModuleLayerOffsetsOut) const;

    // Accessors
    size_t NumScriptLayers() const                   { return mLayerOffsets.size(); }
    uint32 ScriptLayerOffset(size_t LayerIdx) const  { return mLayerOffsets[LayerIdx]; }
};

#endif // CDEPENDENCYTREE

