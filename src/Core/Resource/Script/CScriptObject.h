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

class CScriptObject
{
    friend class CScriptLoader;
    friend class CAreaLoader;

    CScriptTemplate *mpTemplate;
    CGameArea *mpArea;
    CScriptLayer *mpLayer;
    uint32 mVersion;

    uint32 mInstanceID;
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
    uint32 mActiveCharIndex;
    uint32 mActiveAnimIndex;
    bool mHasInGameModel;

    EVolumeShape mVolumeShape;
    float mVolumeScale;

    // Recursion guard
    mutable bool mIsCheckingNearVisibleActivation;

public:
    CScriptObject(uint32 InstanceID, CGameArea *pArea, CScriptLayer *pLayer, CScriptTemplate *pTemplate);
    ~CScriptObject();

    void CopyProperties(CScriptObject* pObject);
    void EvaluateProperties();
    void EvaluateDisplayAsset();
    void EvaluateCollisionModel();
    void EvaluateVolume();
    bool IsEditorProperty(IProperty *pProp);
    void SetLayer(CScriptLayer *pLayer, uint32 NewLayerIndex = -1);
    uint32 LayerIndex() const;
    bool HasNearVisibleActivation() const;

    void AddLink(ELinkType Type, CLink *pLink, uint32 Index = -1);
    void RemoveLink(ELinkType Type, CLink *pLink);
    void BreakAllLinks();

    // Accessors
    CScriptTemplate* Template() const                               { return mpTemplate; }
    CGameTemplate* GameTemplate() const                             { return mpTemplate->GameTemplate(); }
    CGameArea* Area() const                                         { return mpArea; }
    CScriptLayer* Layer() const                                     { return mpLayer; }
    uint32 Version() const                                          { return mVersion; }
    uint32 ObjectTypeID() const                                     { return mpTemplate->ObjectID(); }
    uint32 InstanceID() const                                       { return mInstanceID; }
    uint32 NumLinks(ELinkType Type) const                           { return (Type == ELinkType::Incoming ? mInLinks.size() : mOutLinks.size()); }
    CLink* Link(ELinkType Type, uint32 Index) const                 { return (Type == ELinkType::Incoming ? mInLinks[Index] : mOutLinks[Index]); }
    void* PropertyData() const                                      { return (void*) mPropertyData.data(); }

    CVector3f Position() const                  { return mPosition.IsValid() ? mPosition.Get() : CVector3f::skZero; }
    CVector3f Rotation() const                  { return mRotation.IsValid() ? mRotation.Get() : CVector3f::skZero; }
    CVector3f Scale() const                     { return mScale.IsValid() ? mScale.Get() : CVector3f::skOne; }
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
