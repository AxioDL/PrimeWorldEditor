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
public:
    enum class ERotationType {
        RotationEnabled, RotationDisabled,
    };

    enum class EScaleType {
        ScaleEnabled, ScaleDisabled, ScaleVolume
    };

private:
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

    std::vector<TString> mModules;
    std::unique_ptr<CStructProperty> mpProperties;
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
    uint32 mObjectID;

    // Editor Properties
    TIDString mNameIDString;
    TIDString mPositionIDString;
    TIDString mRotationIDString;
    TIDString mScaleIDString;
    TIDString mActiveIDString;
    TIDString mLightParametersIDString;

    CGameTemplate* mpGame;
    std::list<CScriptObject*> mObjectList;

    CStringProperty* mpNameProperty;
    CVectorProperty* mpPositionProperty;
    CVectorProperty* mpRotationProperty;
    CVectorProperty* mpScaleProperty;
    CBoolProperty* mpActiveProperty;
    CStructProperty* mpLightParametersProperty;

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
    bool mVisible;
    bool mDirty;

public:
    // Default constructor. Don't use. This is only here so the serializer doesn't complain
    CScriptTemplate() { ASSERT(false); }
    // Old constructor
    CScriptTemplate(CGameTemplate *pGame);
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
    inline CGameTemplate* GameTemplate() const              { return mpGame; }
    inline TString Name() const                             { return mpProperties->Name(); }
    inline ERotationType RotationType() const               { return mRotationType; }
    inline EScaleType ScaleType() const                     { return mScaleType; }
    inline float PreviewScale() const                       { return mPreviewScale; }
    inline uint32 ObjectID() const                          { return mObjectID; }
    inline bool IsVisible() const                           { return mVisible; }
    inline TString SourceFile() const                       { return mSourceFile; }
    inline CStructProperty* Properties() const              { return mpProperties.get(); }
    inline uint32 NumAttachments() const                    { return mAttachments.size(); }
    const SAttachment& Attachment(uint32 Index) const       { return mAttachments[Index]; }
    const std::vector<TString>& RequiredModules() const     { return mModules; }

    inline CStringProperty* NameProperty() const                { return mpNameProperty; }
    inline CVectorProperty* PositionProperty() const            { return mpPositionProperty; }
    inline CVectorProperty* RotationProperty() const            { return mpRotationProperty; }
    inline CVectorProperty* ScaleProperty() const               { return mpScaleProperty; }
    inline CBoolProperty* ActiveProperty() const                { return mpActiveProperty; }
    inline CStructProperty* LightParametersProperty() const     { return mpLightParametersProperty; }

    inline void SetVisible(bool Visible)    { mVisible = Visible; }
    inline void MarkDirty()                 { mDirty = true; }
    inline bool IsDirty() const             { return mDirty || mpProperties->IsDirty(); }

    // Object Tracking
    uint32 NumObjects() const;
    const std::list<CScriptObject*>& ObjectList() const;
    void AddObject(CScriptObject *pObject);
    void RemoveObject(CScriptObject *pObject);
    void SortObjects();

private:
    int32 CheckVolumeConditions(CScriptObject *pObj, bool LogErrors);
};

#endif // CSCRIPTTEMPLATE_H
