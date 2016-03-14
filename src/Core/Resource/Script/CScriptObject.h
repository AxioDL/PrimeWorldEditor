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

    CScriptTemplate* Template() const;
    CMasterTemplate* MasterTemplate() const;
    CGameArea* Area() const;
    CScriptLayer* Layer() const;
    u32 Version() const;
    CPropertyStruct* Properties() const;
    u32 NumProperties() const;
    IProperty* PropertyByIndex(u32 index) const;
    IProperty* PropertyByIDString(const TIDString& str) const;
    u32 ObjectTypeID() const;
    u32 InstanceID() const;

    u32 NumLinks(ELinkType Type) const;
    CLink* Link(ELinkType Type, u32 Index) const;
    void AddLink(ELinkType Type, CLink *pLink, u32 Index = -1);
    void RemoveLink(ELinkType Type, CLink *pLink);
    void BreakAllLinks();

    CVector3f Position() const;
    CVector3f Rotation() const;
    CVector3f Scale() const;
    TString InstanceName() const;
    bool IsActive() const;
    bool HasInGameModel() const;
    void SetPosition(const CVector3f& newPos);
    void SetRotation(const CVector3f& newRot);
    void SetScale(const CVector3f& newScale);
    void SetName(const TString& newName);
    void SetActive(bool isActive);
    CPropertyStruct* LightParameters() const;
    CModel* GetDisplayModel() const;
    CTexture* GetBillboard() const;
    CCollisionMeshGroup* GetCollision() const;
    EVolumeShape VolumeShape() const;
    float VolumeScale() const;

    TStringProperty*    InstanceNameProperty() const    { return mpInstanceName; }
    TVector3Property*   PositionProperty() const        { return mpPosition; }
    TVector3Property*   RotationProperty() const        { return mpRotation; }
    TVector3Property*   ScaleProperty() const           { return mpScale; }
    TBoolProperty*      ActiveProperty() const          { return mpActive; }
};

#endif // CSCRIPTOBJECT_H
