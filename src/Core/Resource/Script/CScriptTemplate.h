#ifndef CSCRIPTTEMPLATE_H
#define CSCRIPTTEMPLATE_H

#include "IPropertyTemplate.h"
#include "IProperty.h"
#include "EPropertyType.h"
#include "EVolumeShape.h"
#include "Core/Resource/Model/CModel.h"
#include "Core/Resource/CCollisionMeshGroup.h"
#include <Common/CFourCC.h>
#include <Common/types.h>
#include <list>
#include <vector>

class CMasterTemplate;
class CScriptObject;

typedef TString TIDString;

/*
 * CScriptTemplate is a class that encases the data contained in one of the XML templates.
 * It essentially sets the layout of any given script object.
 *
 * It contains any data that applies globally to every instance of the object, such as
 * property names, editor attribute properties, etc.
 */
class CScriptTemplate
{
    friend class CTemplateLoader;
    friend class CTemplateWriter;

public:
    enum ERotationType {
        eRotationEnabled, eRotationDisabled
    };

    enum EScaleType {
        eScaleEnabled, eScaleDisabled, eScaleVolume
    };

private:
    struct SEditorAsset
    {
        enum {
            eModel, eAnimParams, eBillboard, eCollision
        } AssetType;

        enum {
            eProperty, eFile
        } AssetSource;

        TIDString AssetLocation;
        s32 ForceNodeIndex; // Force animsets to use specific node instead of one from property
    };

    CMasterTemplate *mpMaster;
    CStructTemplate *mpBaseStruct;
    std::list<CScriptObject*> mObjectList;
    TString mTemplateName;
    TString mSourceFile;
    u32 mObjectID;
    bool mVisible;

    // Editor Properties
    TIDString mNameIDString;
    TIDString mPositionIDString;
    TIDString mRotationIDString;
    TIDString mScaleIDString;
    TIDString mActiveIDString;
    TIDString mLightParametersIDString;
    std::vector<SEditorAsset> mAssets;

    ERotationType mRotationType;
    EScaleType mScaleType;
    float mPreviewScale;

    // Preview Volume
    EVolumeShape mVolumeShape;
    float mVolumeScale;
    TIDString mVolumeConditionIDString;

    struct SVolumeCondition {
        int Value;
        EVolumeShape Shape;
        float Scale;
    };
    std::vector<SVolumeCondition> mVolumeConditions;

public:
    CScriptTemplate(CMasterTemplate *pMaster);
    ~CScriptTemplate();

    CMasterTemplate* MasterTemplate();
    EGame Game();
    TString Name() const;
    u32 NumPropertySets() const;
    ERotationType RotationType() const;
    EScaleType ScaleType() const;
    float PreviewScale() const;
    u32 ObjectID() const;
    void SetVisible(bool visible);
    bool IsVisible() const;
    void DebugPrintProperties();

    // Property Fetching
    CStructTemplate* BaseStruct();
    EVolumeShape VolumeShape(CScriptObject *pObj);
    float VolumeScale(CScriptObject *pObj);
    TStringProperty* FindInstanceName(CPropertyStruct *pProperties);
    TVector3Property* FindPosition(CPropertyStruct *pProperties);
    TVector3Property* FindRotation(CPropertyStruct *pProperties);
    TVector3Property* FindScale(CPropertyStruct *pProperties);
    TBoolProperty* FindActive(CPropertyStruct *pProperties);
    CPropertyStruct* FindLightParameters(CPropertyStruct *pProperties);
    CModel* FindDisplayModel(CPropertyStruct *pProperties);
    CTexture* FindBillboardTexture(CPropertyStruct *pProperties);
    CCollisionMeshGroup* FindCollision(CPropertyStruct *pProperties);
    bool HasInGameModel(CPropertyStruct *pProperties);
    bool HasPosition();

    // Object Tracking
    u32 NumObjects() const;
    const std::list<CScriptObject*>& ObjectList() const;
    void AddObject(CScriptObject *pObject);
    void RemoveObject(CScriptObject *pObject);
    void SortObjects();

private:
    s32 CheckVolumeConditions(CScriptObject *pObj, bool LogErrors);
};

#endif // CSCRIPTTEMPLATE_H
