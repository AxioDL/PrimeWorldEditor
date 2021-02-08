#ifndef CSCRIPTOBJECT_H
#define CSCRIPTOBJECT_H

#include "CScriptTemplate.h"
#include "Core/Resource/Area/CGameArea.h"
#include "Core/Resource/Collision/CCollisionMeshGroup.h"
#include "Core/Resource/Model/CModel.h"
#include "Core/Resource/Script/Property/Properties.h"

class CScriptLayer;
class CLink;

enum class ELinkType
{
    Incoming,
    Outgoing
};

class CInstanceID
{
    uint32 mId = 0;
public:
    constexpr operator uint32() const { return mId; }
    constexpr CInstanceID() = default;
    constexpr CInstanceID(uint32 id) : mId(id) {}
    constexpr CInstanceID& operator=(uint32 id) { mId = id; return *this; }
    [[nodiscard]] constexpr uint8 Layer() const { return uint8((mId >> 26u) & 0x3fu); }
    [[nodiscard]] constexpr uint16 Area() const { return uint16((mId >> 16u) & 0x3ffu); }
    [[nodiscard]] constexpr uint16 Id() const { return uint16(mId & 0xffffu); }
};

class CScriptObject
{
    friend class CScriptLoader;
    friend class CAreaLoader;

    CScriptTemplate *mpTemplate;
    CGameArea *mpArea;
    CScriptLayer *mpLayer;
    uint32 mVersion = 0;

    CInstanceID mInstanceID;
    std::vector<CLink*> mOutLinks;
    std::vector<CLink*> mInLinks;
    std::vector<char> mPropertyData;

    CStringRef mInstanceName;
    CVectorRef mPosition;
    CVectorRef mRotation;
    CVectorRef mScale;
    CBoolRef mActive;
    CStructRef mLightParameters;

    TResPtr<CResource> mpDisplayAsset;
    TResPtr<CCollisionMeshGroup> mpCollision;
    uint32 mActiveCharIndex = 0;
    uint32 mActiveAnimIndex = 0;
    bool mHasInGameModel = false;

    EVolumeShape mVolumeShape{};
    float mVolumeScale = 0.0f;

    // Recursion guard
    mutable bool mIsCheckingNearVisibleActivation = false;

public:
    CScriptObject(uint32 InstanceID, CGameArea *pArea, CScriptLayer *pLayer, CScriptTemplate *pTemplate);
    ~CScriptObject();

    void CopyProperties(CScriptObject* pObject);
    void EvaluateProperties();
    void EvaluateDisplayAsset();
    void EvaluateCollisionModel();
    void EvaluateVolume();
    bool IsEditorProperty(const IProperty *pProp) const;
    void SetLayer(CScriptLayer *pLayer, uint32 NewLayerIndex = UINT32_MAX);
    uint32 LayerIndex() const;
    bool HasNearVisibleActivation() const;

    void AddLink(ELinkType Type, CLink *pLink, uint32 Index = UINT32_MAX);
    void RemoveLink(ELinkType Type, CLink *pLink);
    void BreakAllLinks();

    // Accessors
    CScriptTemplate* Template() const                               { return mpTemplate; }
    CGameTemplate* GameTemplate() const                             { return mpTemplate->GameTemplate(); }
    CGameArea* Area() const                                         { return mpArea; }
    CScriptLayer* Layer() const                                     { return mpLayer; }
    uint32 Version() const                                          { return mVersion; }
    uint32 ObjectTypeID() const                                     { return mpTemplate->ObjectID(); }
    CInstanceID InstanceID() const                                  { return mInstanceID; }
    size_t NumLinks(ELinkType Type) const                           { return (Type == ELinkType::Incoming ? mInLinks.size() : mOutLinks.size()); }
    CLink* Link(ELinkType Type, size_t Index) const                 { return (Type == ELinkType::Incoming ? mInLinks[Index] : mOutLinks[Index]); }
    void* PropertyData() const                                      { return (void*) mPropertyData.data(); }

    CVector3f Position() const                  { return mPosition.IsValid() ? mPosition.Get() : CVector3f::Zero(); }
    CVector3f Rotation() const                  { return mRotation.IsValid() ? mRotation.Get() : CVector3f::Zero(); }
    CVector3f Scale() const                     { return mScale.IsValid() ? mScale.Get() : CVector3f::One(); }
    TString InstanceName() const                { return mInstanceName.IsValid() ? mInstanceName.Get() : ""; }
    bool IsActive() const                       { return mActive.IsValid() ? mActive.Get() : false; }
    bool HasInGameModel() const                 { return mHasInGameModel; }
    CStructRef LightParameters() const          { return mLightParameters; }
    CResource* DisplayAsset() const             { return mpDisplayAsset; }
    uint32 ActiveCharIndex() const              { return mActiveCharIndex; }
    uint32 ActiveAnimIndex() const              { return mActiveAnimIndex; }
    CCollisionMeshGroup* Collision() const      { return mpCollision; }
    EVolumeShape VolumeShape() const            { return mVolumeShape; }
    float VolumeScale() const                   { return mVolumeScale; }
    void SetPosition(const CVector3f& rkNewPos) { mPosition.Set(rkNewPos); }
    void SetRotation(const CVector3f& rkNewRot) { mRotation.Set(rkNewRot); }
    void SetScale(const CVector3f& rkNewScale)  { mScale.Set(rkNewScale); }
    void SetName(const TString& rkNewName)      { mInstanceName.Set(rkNewName); }
    void SetActive(bool Active)                 { mActive.Set(Active); }

    bool HasPosition() const        { return mPosition.IsValid(); }
    bool HasRotation() const        { return mRotation.IsValid(); }
    bool HasScale() const           { return mScale.IsValid(); }
    bool HasInstanceName() const    { return mInstanceName.IsValid(); }
    bool HasActive() const          { return mActive.IsValid(); }
    bool HasLightParameters() const { return mLightParameters.IsValid(); }
};

#endif // CSCRIPTOBJECT_H
