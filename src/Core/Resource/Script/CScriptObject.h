#ifndef CSCRIPTOBJECT_H
#define CSCRIPTOBJECT_H

#include "IProperty.h"
#include "IPropertyTemplate.h"
#include "CScriptTemplate.h"
#include "Core/Resource/Model/CModel.h"
#include "Core/Resource/CCollisionMeshGroup.h"
#include "Core/Resource/CGameArea.h"

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
    TResPtr<CGameArea> mpArea;
    CScriptLayer *mpLayer;
    u32 mVersion;

    u32 mInstanceID;
    std::vector<CLink*> mOutLinks;
    std::vector<CLink*> mInLinks;
    CPropertyStruct *mpProperties;

    TStringProperty *mpInstanceName;
    TVector3Property *mpPosition;
    TVector3Property *mpRotation;
    TVector3Property *mpScale;
    TBoolProperty *mpActive;
    CPropertyStruct *mpLightParameters;
    TResPtr<CModel> mpDisplayModel;
    TResPtr<CTexture> mpBillboard;
    TResPtr<CCollisionMeshGroup> mpCollision;
    bool mHasInGameModel;

    EVolumeShape mVolumeShape;
    float mVolumeScale;

    // Recursion guard
    mutable bool mIsCheckingNearVisibleActivation;

public:
    CScriptObject(u32 InstanceID, CGameArea *pArea, CScriptLayer *pLayer, CScriptTemplate *pTemplate);
    ~CScriptObject();

    void EvaluateProperties();
    void EvaluateDisplayModel();
    void EvaluateBillboard();
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
    CMasterTemplate* MasterTemplate() const                         { return mpTemplate->MasterTemplate(); }
    CGameArea* Area() const                                         { return mpArea; }
    CScriptLayer* Layer() const                                     { return mpLayer; }
    u32 Version() const                                             { return mVersion; }
    CPropertyStruct* Properties() const                             { return mpProperties; }
    u32 NumProperties() const                                       { return mpProperties->Count(); }
    IProperty* PropertyByIndex(u32 Index) const                     { return mpProperties->PropertyByIndex(Index); }
    IProperty* PropertyByIDString(const TIDString& rkStr) const     { return mpProperties->PropertyByIDString(rkStr); }
    u32 ObjectTypeID() const                                        { return mpTemplate->ObjectID(); }
    u32 InstanceID() const                                          { return mInstanceID; }
    u32 NumLinks(ELinkType Type) const                              { return (Type == eIncoming ? mInLinks.size() : mOutLinks.size()); }
    CLink* Link(ELinkType Type, u32 Index) const                    { return (Type == eIncoming ? mInLinks[Index] : mOutLinks[Index]); }

    CVector3f Position() const                  { return mpPosition ? mpPosition->Get() : CVector3f::skZero; }
    CVector3f Rotation() const                  { return mpRotation ? mpRotation->Get() : CVector3f::skZero; }
    CVector3f Scale() const                     { return mpScale ? mpScale->Get() : CVector3f::skZero; }
    TString InstanceName() const                { return mpInstanceName ? mpInstanceName->Get() : ""; }
    bool IsActive() const                       { return mpActive ? mpActive->Get() : false; }
    bool HasInGameModel() const                 { return mHasInGameModel; }
    void SetPosition(const CVector3f& rkNewPos) { if (mpPosition) mpPosition->Set(rkNewPos); }
    void SetRotation(const CVector3f& rkNewRot) { if (mpRotation) mpRotation->Set(rkNewRot); }
    void SetScale(const CVector3f& rkNewScale)  { if (mpScale) mpScale->Set(rkNewScale); }
    void SetName(const TString& rkNewName)      { if (mpInstanceName) mpInstanceName->Set(rkNewName); }
    void SetActive(bool Active)                 { if (mpActive) mpActive->Set(Active); }
    CPropertyStruct* LightParameters() const    { return mpLightParameters; }
    CModel* GetDisplayModel() const             { return mpDisplayModel; }
    CTexture* GetBillboard() const              { return mpBillboard; }
    CCollisionMeshGroup* GetCollision() const   { return mpCollision; }
    EVolumeShape VolumeShape() const            { return mVolumeShape; }
    float VolumeScale() const                   { return mVolumeScale; }

    TStringProperty*    InstanceNameProperty() const    { return mpInstanceName; }
    TVector3Property*   PositionProperty() const        { return mpPosition; }
    TVector3Property*   RotationProperty() const        { return mpRotation; }
    TVector3Property*   ScaleProperty() const           { return mpScale; }
    TBoolProperty*      ActiveProperty() const          { return mpActive; }
};

#endif // CSCRIPTOBJECT_H
