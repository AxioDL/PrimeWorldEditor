#ifndef CSCRIPTOBJECT_H
#define CSCRIPTOBJECT_H

#include "SConnection.h"
#include "CProperty.h"
#include "CScriptTemplate.h"
#include "EAttribType.h"
#include "../model/CModel.h"

class CGameArea;
class CScriptLayer;

class CScriptObject
{
    friend class CScriptLoader;
    friend class CAreaLoader;

    CScriptTemplate *mpTemplate;
    CGameArea *mpArea;
    CScriptLayer *mpLayer;

    u32 mInstanceID;
    std::vector<SLink> mOutConnections;
    std::vector<SLink> mInConnections;
    CPropertyStruct *mpProperties;

    CVector3f mPosition, mRotation, mScale;
    CVector3f mVolumeSize;
    u32 mVolumeShape;
    std::string mInstanceName;
    CColor mTevColor;
    CModel* mpDisplayModel;

    struct SAttrib
    {
        EAttribType Type;
        u32 Settings;
        CResource *Res;
        CToken ResToken;
        CPropertyBase *Prop;

        // Convenience constructor
        SAttrib(EAttribType type, CResource *res, u32 settings, CPropertyBase *prop) {
            Type = type;
            Res = res;
            ResToken = CToken(res);
            Settings = settings;
            Prop = prop;
        }
    };
    std::vector<SAttrib> mAttribs;

    int mAttribFlags; // int container for EAttribType flags

public:
    CScriptObject(CGameArea *pArea, CScriptLayer *pLayer, CScriptTemplate *pTemplate);
    ~CScriptObject();

    void EvaluateDisplayModel();
    void EvaluateInstanceName();
    void EvaluateTevColor();
    void EvalutateXForm();

    CScriptTemplate* Template();
    CMasterTemplate* MasterTemplate();
    CGameArea* Area();
    CScriptLayer* Layer();
    CPropertyStruct* Properties();
    u32 ObjectTypeID() const;
    u32 InstanceID() const;
    u32 NumInLinks() const;
    u32 NumOutLinks() const;
    const SLink& InLink(u32 index) const;
    const SLink& OutLink(u32 index) const;

    CPropertyBase* PropertyByIndex(u32 index);
    CPropertyBase* PropertyByName(std::string name);

    CVector3f GetPosition() const;
    CVector3f GetRotation() const;
    CVector3f GetScale() const;
    CVector3f GetVolume() const;
    u32 GetVolumeShape() const;
    std::string GetInstanceName() const;
    CColor GetTevColor() const;
    CModel* GetDisplayModel() const;
    int GetAttribFlags() const;

    // Static
    static CScriptObject* CopyFromTemplate(CScriptTemplate *pTemp, CGameArea *pArea, CScriptLayer *pLayer);
};

#endif // CSCRIPTOBJECT_H
