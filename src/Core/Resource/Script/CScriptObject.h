#ifndef CSCRIPTOBJECT_H
#define CSCRIPTOBJECT_H

#include "SConnection.h"
#include "CProperty.h"
#include "CPropertyTemplate.h"
#include "CScriptTemplate.h"
#include "Core/Resource/Model/CModel.h"
#include "Core/Resource/CCollisionMeshGroup.h"
#include "Core/Resource/CGameArea.h"

class CScriptLayer;

class CScriptObject
{
    friend class CScriptLoader;
    friend class CAreaLoader;

    CScriptTemplate *mpTemplate;
    TResPtr<CGameArea> mpArea;
    CScriptLayer *mpLayer;

    u32 mInstanceID;
    std::vector<SLink> mOutConnections;
    std::vector<SLink> mInConnections;
    CPropertyStruct *mpProperties;

    CStringProperty *mpInstanceName;
    CVector3Property *mpPosition;
    CVector3Property *mpRotation;
    CVector3Property *mpScale;
    CBoolProperty *mpActive;
    CPropertyStruct *mpLightParameters;
    TResPtr<CModel> mpDisplayModel;
    TResPtr<CTexture> mpBillboard;
    TResPtr<CCollisionMeshGroup> mpCollision;
    bool mHasInGameModel;

    EVolumeShape mVolumeShape;
    float mVolumeScale;

public:
    CScriptObject(CGameArea *pArea, CScriptLayer *pLayer, CScriptTemplate *pTemplate);
    ~CScriptObject();

    void CopyFromTemplate(CScriptTemplate *pTemp, u32 propCount);
    void EvaluateProperties();
    void EvaluateDisplayModel();
    void EvaluateBillboard();
    void EvaluateCollisionModel();

    CScriptTemplate* Template() const;
    CMasterTemplate* MasterTemplate() const;
    CGameArea* Area() const;
    CScriptLayer* Layer() const;
    CPropertyStruct* Properties() const;
    u32 NumProperties() const;
    CPropertyBase* PropertyByIndex(u32 index) const;
    CPropertyBase* PropertyByIDString(const TIDString& str) const;
    u32 ObjectTypeID() const;
    u32 InstanceID() const;
    u32 NumInLinks() const;
    u32 NumOutLinks() const;
    const SLink& InLink(u32 index) const;
    const SLink& OutLink(u32 index) const;

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
};

#endif // CSCRIPTOBJECT_H
