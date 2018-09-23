#ifndef CSCRIPTOBJECT_H
#define CSCRIPTOBJECT_H

#include "CScriptTemplate.h"
#include "Core/Resource/Area/CGameArea.h"
#include "Core/Resource/Model/CModel.h"
#include "Core/Resource/CCollisionMeshGroup.h"
#include "Core/Resource/Script/Property/Properties.h"

class CScriptLayer;
class CLink;

enum ELinkType
{
    eIncoming,
    eOutgoing
};

class CScriptObject
{
    friend class CScriptLoader;
    friend class CAreaLoader;

    CScriptTemplate *mpTemplate;
    CGameArea *mpArea;
    CScriptLayer *mpLayer;
    u32 mVersion;

    u32 mInstanceID;
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
    u32 mActiveCharIndex;
    u32 mActiveAnimIndex;
    bool mHasInGameModel;

    EVolumeShape mVolumeShape;
    float mVolumeScale;

    // Recursion guard
    mutable bool mIsCheckingNearVisibleActivation;

public:
    CScriptObject(u32 InstanceID, CGameArea *pArea, CScriptLayer *pLayer, CScriptTemplate *pTemplate);
    ~CScriptObject();

    void CopyProperties(CScriptObject* pObject);
    void EvaluateProperties();
    void EvaluateDisplayAsset();
    void EvaluateCollisionModel();
    void EvaluateVolume();
    bool IsEditorProperty(IProperty *pProp);
    void SetLayer(CScriptLayer *pLayer, u32 NewLayerIndex = -1);
    u32 LayerIndex() const;
    bool HasNearVisibleActivation() const;

    void AddLink(ELinkType Type, CLink *pLink, u32 Index = -1);
    void RemoveLink(ELinkType Type, CLink *pLink);
    void BreakAllLinks();

    // Accessors
    CScriptTemplate* Template() const                               { return mpTemplate; }
    CGameTemplate* GameTemplate() const                             { return mpTemplate->GameTemplate(); }
    CGameArea* Area() const                                         { return mpArea; }
    CScriptLayer* Layer() const                                     { return mpLayer; }
    u32 Version() const                                             { return mVersion; }
    u32 ObjectTypeID() const                                        { return mpTemplate->ObjectID(); }
    u32 InstanceID() const                                          { return mInstanceID; }
    u32 NumLinks(ELinkType Type) const                              { return (Type == eIncoming ? mInLinks.size() : mOutLinks.size()); }
    CLink* Link(ELinkType Type, u32 Index) const                    { return (Type == eIncoming ? mInLinks[Index] : mOutLinks[Index]); }
    void* PropertyData() const                                      { return (void*) mPropertyData.data(); }

    CVector3f Position() const                  { return mPosition.IsValid() ? mPosition.Get() : CVector3f::skZero; }
    CVector3f Rotation() const                  { return mRotation.IsValid() ? mRotation.Get() : CVector3f::skZero; }
    CVector3f Scale() const                     { return mScale.IsValid() ? mScale.Get() : CVector3f::skOne; }
    TString InstanceName() const                { return mInstanceName.IsValid() ? mInstanceName.Get() : ""; }
    bool IsActive() const                       { return mActive.IsValid() ? mActive.Get() : false; }
    bool HasInGameModel() const                 { return mHasInGameModel; }
    CStructRef LightParameters() const          { return mLightParameters; }
    CResource* DisplayAsset() const             { return mpDisplayAsset; }
    u32 ActiveCharIndex() const                 { return mActiveCharIndex; }
    u32 ActiveAnimIndex() const                 { return mActiveAnimIndex; }
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
