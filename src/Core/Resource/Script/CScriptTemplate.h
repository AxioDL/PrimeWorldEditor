#ifndef CSCRIPTTEMPLATE_H
#define CSCRIPTTEMPLATE_H

#include "Core/Resource/Script/Property/Properties.h"
#include "EVolumeShape.h"
#include "Core/Resource/Model/CModel.h"
#include "Core/Resource/Collision/CCollisionMeshGroup.h"
#include <Common/BasicTypes.h>
#include <Common/CFourCC.h>
#include <list>
#include <vector>

class CGameTemplate;
class CScriptObject;
typedef TString TIDString;

enum class EAttachType {
    Attach,
    Follow
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
public:
    enum class ERotationType {
        RotationEnabled, RotationDisabled,
    };

    enum class EScaleType {
        ScaleEnabled, ScaleDisabled, ScaleVolume
    };

    struct SEditorAsset
    {
        enum class EAssetType {
            Model, AnimParams, Billboard, Collision
        } AssetType;

        enum class EAssetSource {
            Property, File
        } AssetSource;

        TIDString AssetLocation;
        int32 ForceNodeIndex; // Force animsets to use specific node instead of one from property

        void Serialize(IArchive& Arc)
        {
            Arc << SerialParameter("Type", AssetType, SH_Attribute)
                << SerialParameter("Source", AssetSource, SH_Attribute)
                << SerialParameter("Location", AssetLocation, SH_Attribute)
                << SerialParameter("ForceCharacterIndex", ForceNodeIndex, SH_Attribute | SH_Optional, (int32) -1);
        }
    };

private:
    std::vector<TString> mModules;
    std::unique_ptr<CStructProperty> mpProperties;
    std::vector<SEditorAsset> mAssets;
    std::vector<SAttachment> mAttachments;

    ERotationType mRotationType{ERotationType::RotationEnabled};
    EScaleType mScaleType{EScaleType::ScaleEnabled};
    float mPreviewScale = 1.0f;

    // Preview Volume
    EVolumeShape mVolumeShape{EVolumeShape::NoShape};
    float mVolumeScale = 1.0f;
    TIDString mVolumeConditionIDString;

    TString mSourceFile;
    uint32 mObjectID = 0;

    // Editor Properties
    TIDString mNameIDString;
    TIDString mPositionIDString;
    TIDString mRotationIDString;
    TIDString mScaleIDString;
    TIDString mActiveIDString;
    TIDString mLightParametersIDString;

    CGameTemplate* mpGame;
    std::list<CScriptObject*> mObjectList;

    CStringProperty* mpNameProperty = nullptr;
    CVectorProperty* mpPositionProperty = nullptr;
    CVectorProperty* mpRotationProperty = nullptr;
    CVectorProperty* mpScaleProperty = nullptr;
    CBoolProperty* mpActiveProperty = nullptr;
    CStructProperty* mpLightParametersProperty = nullptr;

    struct SVolumeCondition {
        uint32 Value;
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
    bool mVisible = true;
    bool mDirty = false;

public:
    // Default constructor. Don't use. This is only here so the serializer doesn't complain
    CScriptTemplate() { ASSERT(false); }
    // Old constructor
    explicit CScriptTemplate(CGameTemplate *pGame);
    // New constructor
    CScriptTemplate(CGameTemplate* pGame, uint32 ObjectID, const TString& kFilePath);
    ~CScriptTemplate();
    void Serialize(IArchive& rArc);
    void Save(bool Force = false);
    EGame Game() const;

    // Property Fetching
    EVolumeShape VolumeShape(CScriptObject *pObj);
    float VolumeScale(CScriptObject *pObj);
    CResource* FindDisplayAsset(void* pPropertyData, uint32& rOutCharIndex, uint32& rOutAnimIndex, bool& rOutIsInGame);
    CCollisionMeshGroup* FindCollision(void* pPropertyData);

    // Accessors
    CGameTemplate* GameTemplate() const              { return mpGame; }
    TString Name() const                             { return mpProperties->Name(); }
    ERotationType RotationType() const               { return mRotationType; }
    EScaleType ScaleType() const                     { return mScaleType; }
    float PreviewScale() const                       { return mPreviewScale; }
    uint32 ObjectID() const                          { return mObjectID; }
    bool IsVisible() const                           { return mVisible; }
    TString SourceFile() const                       { return mSourceFile; }
    CStructProperty* Properties() const              { return mpProperties.get(); }
    size_t NumAttachments() const                    { return mAttachments.size(); }
    const SAttachment& Attachment(size_t Index) const    { return mAttachments[Index]; }
    const std::vector<TString>& RequiredModules() const  { return mModules; }

    CStringProperty* NameProperty() const                { return mpNameProperty; }
    CVectorProperty* PositionProperty() const            { return mpPositionProperty; }
    CVectorProperty* RotationProperty() const            { return mpRotationProperty; }
    CVectorProperty* ScaleProperty() const               { return mpScaleProperty; }
    CBoolProperty* ActiveProperty() const                { return mpActiveProperty; }
    CStructProperty* LightParametersProperty() const     { return mpLightParametersProperty; }

    void SetVisible(bool Visible)    { mVisible = Visible; }
    void MarkDirty()                 { mDirty = true; }
    bool IsDirty() const             { return mDirty || mpProperties->IsDirty(); }

    // Object Tracking
    uint32 NumObjects() const;
    const std::list<CScriptObject*>& ObjectList() const;
    void AddObject(CScriptObject *pObject);
    void RemoveObject(const CScriptObject *pObject);
    void SortObjects();

private:
    int32 CheckVolumeConditions(CScriptObject *pObj, bool LogErrors);
};

#endif // CSCRIPTTEMPLATE_H
