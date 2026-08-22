#ifndef CSCRIPTTEMPLATE_H
#define CSCRIPTTEMPLATE_H

#include <Common/EGame.h>
#include <Common/EnumReflection.h>
#include <Common/TString.h>
#include <Common/Serialization/IArchive.h>
#include "Core/Resource/Script/EVolumeShape.h"

#include <list>
#include <memory>
#include <ranges>
#include <span>
#include <vector>

class CBoolProperty;
class CCollisionMeshGroup;
class CGameTemplate;
class CResource;
class CResourceStore;
class CScriptObject;
class CStringProperty;
class CStructProperty;
class CVectorProperty;

using TIDString = TString;

enum class EAttachType {
    Attach,
    Follow
};

struct SAttachment
{
    TIDString AttachProperty; // Must point to a CMDL!
    TString LocatorName;
    EAttachType AttachType{};

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
        } AssetType{};

        enum class EAssetSource {
            Property, File
        } AssetSource{};

        TIDString AssetLocation;
        int32_t ForceNodeIndex{}; // Force animsets to use specific node instead of one from property

        void Serialize(IArchive& Arc)
        {
            Arc << SerialParameter("Type", AssetType, SH_Attribute)
                << SerialParameter("Source", AssetSource, SH_Attribute)
                << SerialParameter("Location", AssetLocation, SH_Attribute)
                << SerialParameter("ForceCharacterIndex", ForceNodeIndex, SH_Attribute | SH_Optional, (int32_t) -1);
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
    uint32_t mObjectID = 0;

    // Editor Properties
    TIDString mNameIDString;
    TIDString mPositionIDString;
    TIDString mRotationIDString;
    TIDString mScaleIDString;
    TIDString mActiveIDString;
    TIDString mLightParametersIDString;

    CGameTemplate* mpGame = nullptr;
    std::list<CScriptObject*> mObjectList;

    CStringProperty* mpNameProperty = nullptr;
    CVectorProperty* mpPositionProperty = nullptr;
    CVectorProperty* mpRotationProperty = nullptr;
    CVectorProperty* mpScaleProperty = nullptr;
    CBoolProperty* mpActiveProperty = nullptr;
    CStructProperty* mpLightParametersProperty = nullptr;

    struct SVolumeCondition {
        uint32_t Value{};
        EVolumeShape Shape{};
        float Scale{};

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
    CScriptTemplate();
    // Old constructor
    explicit CScriptTemplate(CGameTemplate *pGame);
    // New constructor
    CScriptTemplate(CGameTemplate* pGame, uint32_t ObjectID, const TString& kFilePath);
    ~CScriptTemplate();
    void Serialize(IArchive& rArc);
    void Save(bool Force = false);
    EGame Game() const;

    // Property Fetching
    EVolumeShape VolumeShape(CScriptObject* pObj) const;
    float VolumeScale(CScriptObject* pObj) const;
    CResource* FindDisplayAsset(CResourceStore* resourceStore, const void* pPropertyData, uint32_t& rOutCharIndex,
                                uint32_t& rOutAnimIndex, bool& rOutIsInGame);
    CCollisionMeshGroup* FindCollision(CResourceStore* resourceStore, const void* pPropertyData);

    // Accessors
    CGameTemplate* GameTemplate() const              { return mpGame; }
    const TString& Name() const;
    ERotationType RotationType() const               { return mRotationType; }
    EScaleType ScaleType() const                     { return mScaleType; }
    float PreviewScale() const                       { return mPreviewScale; }
    uint32_t ObjectID() const                        { return mObjectID; }
    bool IsVisible() const                           { return mVisible; }
    const TString& SourceFile() const                { return mSourceFile; }
    CStructProperty* Properties() const              { return mpProperties.get(); }
    auto Attachments() const                         { return std::views::all(mAttachments); }
    std::span<const TString> RequiredModules() const { return mModules; }

    CStringProperty* NameProperty() const            { return mpNameProperty; }
    CVectorProperty* PositionProperty() const        { return mpPositionProperty; }
    CVectorProperty* RotationProperty() const        { return mpRotationProperty; }
    CVectorProperty* ScaleProperty() const           { return mpScaleProperty; }
    CBoolProperty* ActiveProperty() const            { return mpActiveProperty; }
    CStructProperty* LightParametersProperty() const { return mpLightParametersProperty; }

    void SetVisible(bool Visible) { mVisible = Visible; }
    void MarkDirty()              { mDirty = true; }
    bool IsDirty() const;

    // Object Tracking
    uint32_t NumObjects() const;
    const std::list<CScriptObject*>& ObjectList() const;
    void AddObject(CScriptObject *pObject);
    void RemoveObject(const CScriptObject *pObject);
    void SortObjects();

private:
    int CheckVolumeConditions(CScriptObject *pObj, bool LogErrors) const;
};

#endif // CSCRIPTTEMPLATE_H
