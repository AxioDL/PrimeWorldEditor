#ifndef CSCRIPTTEMPLATE_H
#define CSCRIPTTEMPLATE_H

#include "Core/Resource/Script/Property/Properties.h"
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

enum EAttachType
{
    eAttach,
    eFollow
};

struct SAttachment
{
    TIDString AttachProperty; // Must point to a CMDL!
    TString LocatorName;
    EAttachType AttachType;

    void Serialize(IArchive& Arc)
    {
        Arc << SerialParameter("AttachProperty", AttachProperty, SH_Attribute)
            << SerialParameter("LocatorName", LocatorName, SH_Attribute)
            << SerialParameter("AttachType", AttachType, SH_Attribute);
    }
};

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
        enum EAssetType {
            eModel, eAnimParams, eBillboard, eCollision
        } AssetType;

        enum EAssetSource {
            eProperty, eFile
        } AssetSource;

        TIDString AssetLocation;
        s32 ForceNodeIndex; // Force animsets to use specific node instead of one from property

        void Serialize(IArchive& Arc)
        {
            Arc << SerialParameter("Type", AssetType, SH_Attribute)
                << SerialParameter("Source", AssetSource, SH_Attribute)
                << SerialParameter("Location", AssetLocation, SH_Attribute)
                << SerialParameter("ForceCharacterIndex", ForceNodeIndex, SH_Attribute | SH_Optional, (s32) -1);
        }
    };

    std::vector<TString> mModules;
    std::unique_ptr<CStructPropertyNew> mpProperties;
    std::vector<SEditorAsset> mAssets;
    std::vector<SAttachment> mAttachments;

    ERotationType mRotationType;
    EScaleType mScaleType;
    float mPreviewScale;

    // Preview Volume
    EVolumeShape mVolumeShape;
    float mVolumeScale;
    TIDString mVolumeConditionIDString;

    TString mSourceFile;
    u32 mObjectID;

    // Editor Properties
    TIDString mNameIDString;
    TIDString mPositionIDString;
    TIDString mRotationIDString;
    TIDString mScaleIDString;
    TIDString mActiveIDString;
    TIDString mLightParametersIDString;

    CMasterTemplate* mpMaster;
    std::list<CScriptObject*> mObjectList;

    CStringProperty* mpNameProperty;
    CVectorProperty* mpPositionProperty;
    CVectorProperty* mpRotationProperty;
    CVectorProperty* mpScaleProperty;
    CBoolProperty* mpActiveProperty;
    CStructPropertyNew* mpLightParametersProperty;

    struct SVolumeCondition {
        u32 Value;
        EVolumeShape Shape;
        float Scale;

        void Serialize(IArchive& Arc)
        {
            Arc << SerialParameter("Value", Value)
                << SerialParameter("Shape", Shape)
                << SerialParameter("Scale", Scale, SH_Optional, 1.0f);
        }
    };
    std::vector<SVolumeCondition> mVolumeConditions;
    bool mVisible;

public:
    // Old constructor
    CScriptTemplate(CMasterTemplate *pMaster);
    // New constructor
    CScriptTemplate(CMasterTemplate* pMaster, u32 ObjectID, const TString& kFilePath);
    ~CScriptTemplate();
    void Serialize(IArchive& rArc);
    void PostLoad();
    EGame Game() const;

    // Property Fetching
    EVolumeShape VolumeShape(CScriptObject *pObj);
    float VolumeScale(CScriptObject *pObj);
    CResource* FindDisplayAsset(void* pPropertyData, u32& rOutCharIndex, u32& rOutAnimIndex, bool& rOutIsInGame);
    CCollisionMeshGroup* FindCollision(void* pPropertyData);

    // Accessors
    inline CMasterTemplate* MasterTemplate() const          { return mpMaster; }
    inline TString Name() const                             { return mpProperties->Name(); }
    inline ERotationType RotationType() const               { return mRotationType; }
    inline EScaleType ScaleType() const                     { return mScaleType; }
    inline float PreviewScale() const                       { return mPreviewScale; }
    inline u32 ObjectID() const                             { return mObjectID; }
    inline bool IsVisible() const                           { return mVisible; }
    inline TString SourceFile() const                       { return mSourceFile; }
    inline CStructPropertyNew* Properties() const           { return mpProperties.get(); }
    inline u32 NumAttachments() const                       { return mAttachments.size(); }
    const SAttachment& Attachment(u32 Index) const          { return mAttachments[Index]; }
    const std::vector<TString>& RequiredModules() const     { return mModules; }

    inline CStringProperty* NameProperty() const                { return mpNameProperty; }
    inline CVectorProperty* PositionProperty() const            { return mpPositionProperty; }
    inline CVectorProperty* RotationProperty() const            { return mpRotationProperty; }
    inline CVectorProperty* ScaleProperty() const               { return mpScaleProperty; }
    inline CBoolProperty* ActiveProperty() const                { return mpActiveProperty; }
    inline CStructPropertyNew* LightParametersProperty() const  { return mpLightParametersProperty; }

    inline void SetVisible(bool Visible)    { mVisible = Visible; }

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
