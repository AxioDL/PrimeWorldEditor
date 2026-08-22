#include "Core/Resource/Script/CScriptTemplate.h"

#include <Common/Serialization/CXMLReader.h>
#include <Common/Serialization/CXMLWriter.h>
#include "Core/GameProject/CResourceStore.h"
#include "Core/Resource/Animation/CAnimSet.h"
#include "Core/Resource/Script/CGameTemplate.h"
#include "Core/Resource/Script/CScriptObject.h"
#include "Core/Resource/Script/Property/Properties.h"

#include <algorithm>

CScriptTemplate::CScriptTemplate()
{
    ASSERT(false);
}

// Old constructor
CScriptTemplate::CScriptTemplate(CGameTemplate *pGame)
    : mpGame(pGame)
{
}

// New constructor
CScriptTemplate::CScriptTemplate(CGameTemplate* pInGame, uint32_t InObjectID, const TString& kInFilePath)
    : mSourceFile(kInFilePath)
    , mObjectID(InObjectID)
    , mpGame(pInGame)
{
    // Load
    CXMLReader Reader(kInFilePath);
    ASSERT(Reader.IsValid());
    Serialize(Reader);

    // Post load initialization
    mSourceFile = kInFilePath;
    mpProperties->Initialize(nullptr, this, 0);

    if (!mNameIDString.IsEmpty())               mpNameProperty = TPropCast<CStringProperty>( mpProperties->ChildByIDString(mNameIDString) );
    if (!mPositionIDString.IsEmpty())           mpPositionProperty = TPropCast<CVectorProperty>( mpProperties->ChildByIDString(mPositionIDString) );
    if (!mRotationIDString.IsEmpty())           mpRotationProperty = TPropCast<CVectorProperty>( mpProperties->ChildByIDString(mRotationIDString) );
    if (!mScaleIDString.IsEmpty())              mpScaleProperty = TPropCast<CVectorProperty>( mpProperties->ChildByIDString(mScaleIDString) );
    if (!mActiveIDString.IsEmpty())             mpActiveProperty = TPropCast<CBoolProperty>( mpProperties->ChildByIDString(mActiveIDString) );
    if (!mLightParametersIDString.IsEmpty())    mpLightParametersProperty = TPropCast<CStructProperty>( mpProperties->ChildByIDString(mLightParametersIDString) );
}

CScriptTemplate::~CScriptTemplate() = default;

void CScriptTemplate::Serialize(IArchive& Arc)
{
    Arc << SerialParameter("Modules", mModules, SH_Optional)
        << SerialParameter("Properties", mpProperties);

    if (Arc.ParamBegin("EditorProperties", 0))
    {
        Arc << SerialParameter("NameProperty", mNameIDString, SH_Optional)
            << SerialParameter("PositionProperty", mPositionIDString, SH_Optional)
            << SerialParameter("RotationProperty", mRotationIDString, SH_Optional)
            << SerialParameter("ScaleProperty", mScaleIDString, SH_Optional)
            << SerialParameter("ActiveProperty", mActiveIDString, SH_Optional)
            << SerialParameter("LightParametersProperty", mLightParametersIDString, SH_Optional);

        Arc.ParamEnd();
    }

    Arc << SerialParameter("Assets", mAssets, SH_Optional)
        << SerialParameter("Attachments", mAttachments, SH_Optional)
        << SerialParameter("RotationType", mRotationType, SH_Optional, ERotationType::RotationEnabled)
        << SerialParameter("ScaleType", mScaleType, SH_Optional, EScaleType::ScaleEnabled)
        << SerialParameter("PreviewScale", mPreviewScale, SH_Optional, 1.0f)
        << SerialParameter("VolumeShape", mVolumeShape, SH_Optional, EVolumeShape::NoShape)
        << SerialParameter("VolumeScale", mVolumeScale, SH_Optional, 1.0f)
        << SerialParameter("VolumeConditionProperty", mVolumeConditionIDString, SH_Optional)
        << SerialParameter("VolumeConditions", mVolumeConditions, SH_Optional);
}

void CScriptTemplate::Save(bool Force)
{
    if (IsDirty() || Force)
    {
        NLog::Debug("Saving script template: {}", mSourceFile);
        CXMLWriter Writer(mSourceFile, "ScriptObject", 0, mpGame->Game());
        ASSERT(Writer.IsValid());
        Serialize(Writer);
        mDirty = false;
    }
}

EGame CScriptTemplate::Game() const
{
    return mpGame->Game();
}

EVolumeShape CScriptTemplate::VolumeShape(CScriptObject *pObj)
{
    if (pObj->Template() != this)
    {
        NLog::Error("{} instance somehow called VolumeShape() on {} template", pObj->Template()->Name(), Name());
        return EVolumeShape::InvalidShape;
    }

    if (mVolumeShape == EVolumeShape::ConditionalShape)
    {
        const auto Index = CheckVolumeConditions(pObj, true);
        if (Index == -1)
            return EVolumeShape::InvalidShape;

        return mVolumeConditions[Index].Shape;
    }

    return mVolumeShape;
}

float CScriptTemplate::VolumeScale(CScriptObject *pObj)
{
    if (pObj->Template() != this)
    {
        NLog::Error("{} instance somehow called VolumeScale() on {} template", pObj->Template()->Name(), Name());
        return -1;
    }

    if (mVolumeShape == EVolumeShape::ConditionalShape)
    {
        const auto Index = CheckVolumeConditions(pObj, false);
        if (Index == -1)
            return mVolumeScale;

        return mVolumeConditions[Index].Scale;
    }

    return mVolumeScale;
}

int CScriptTemplate::CheckVolumeConditions(CScriptObject *pObj, bool LogErrors) const
{
    // Private function
    if (mVolumeShape == EVolumeShape::ConditionalShape)
    {
        const TIDString PropID = mVolumeConditionIDString;
        IProperty* pProp = pObj->Template()->Properties()->ChildByIDString(PropID);

        // Get value of the condition test property (only boolean, integral, and enum types supported)
        void* pData = pObj->PropertyData();
        int Val = 0;

        switch (pProp->Type())
        {
        case EPropertyType::Bool:
            Val = TPropCast<CBoolProperty>(pProp)->Value(pData) ? 1 : 0;
            break;

        case EPropertyType::Byte:
            Val = static_cast<int>(TPropCast<CByteProperty>(pProp)->Value(pData));
            break;

        case EPropertyType::Short:
            Val = static_cast<int>(TPropCast<CShortProperty>(pProp)->Value(pData));
            break;

        case EPropertyType::Int:
            Val = TPropCast<CIntProperty>(pProp)->Value(pData);
            break;

        case EPropertyType::Enum:
        case EPropertyType::Choice:
            Val = TPropCast<CEnumProperty>(pProp)->Value(pData);
            break;
        default:
            break;
        }

        // Test and check whether any of the conditions are true
        for (uint32_t LinkIdx = 0; LinkIdx < mVolumeConditions.size(); LinkIdx++)
        {
            if (mVolumeConditions[LinkIdx].Value == Val)
                return LinkIdx;
        }

        if (LogErrors)
        {
            NLog::Error("{} instance {:08X} has unexpected volume shape value of 0x{:X}", pObj->Template()->Name(),
                        pObj->InstanceID(), Val);
        }
    }

    return -1;
}

CResource* CScriptTemplate::FindDisplayAsset(CResourceStore* resourceStore, const void* pPropertyData, uint32_t& rOutCharIndex,
                                             uint32_t& rOutAnimIndex, bool& rOutIsInGame)
{
    rOutCharIndex = UINT32_MAX;
    rOutAnimIndex = UINT32_MAX;
    rOutIsInGame = false;

    for (const auto& asset : mAssets)
    {
        if (asset.AssetType == SEditorAsset::EAssetType::Collision)
            continue;

        CResource *pRes = nullptr;

        // File
        if (asset.AssetSource == SEditorAsset::EAssetSource::File)
        {
            pRes = gpEditorStore->LoadResource(asset.AssetLocation);
        }
        else // Property
        {
            IProperty* pProp = mpProperties->ChildByIDString(asset.AssetLocation);

            if (asset.AssetType == SEditorAsset::EAssetType::AnimParams && pProp->Type() == EPropertyType::AnimationSet)
            {
                const auto* pAnimSet = TPropCast<CAnimationSetProperty>(pProp);
                const auto& Params = pAnimSet->ValueRef(pPropertyData);
                pRes = Params.AnimSet();

                if (pRes != nullptr)
                {
                    const size_t MaxNumChars = static_cast<const CAnimSet*>(pRes)->NumCharacters();
                    rOutCharIndex = (asset.ForceNodeIndex >= 0 && asset.ForceNodeIndex < static_cast<int>(MaxNumChars) ? asset.ForceNodeIndex : Params.CharacterIndex());
                    rOutAnimIndex = Params.AnimIndex();
                }
            }
            else
            {
                ASSERT(pProp->Type() == EPropertyType::Asset);
                const auto* pAsset = TPropCast<CAssetProperty>(pProp);
                const auto& ID = pAsset->ValueRef(pPropertyData);
                if (CResourceEntry* pEntry = resourceStore->FindEntry(ID))
                    pRes = pEntry->Load();
            }
        }

        // If we have a valid resource, return
        if (pRes != nullptr)
        {
            rOutIsInGame = (pRes->Type() != EResourceType::Texture && asset.AssetSource == SEditorAsset::EAssetSource::Property);
            return pRes;
        }
    }

    // None are valid - no display asset
    return nullptr;
}

CCollisionMeshGroup* CScriptTemplate::FindCollision(CResourceStore* resourceStore, const void* pPropertyData)
{
    for (const auto& asset : mAssets)
    {
        if (asset.AssetType != SEditorAsset::EAssetType::Collision)
            continue;

        CResource *pRes = nullptr;

        // File
        if (asset.AssetSource == SEditorAsset::EAssetSource::File)
        {
            pRes = resourceStore->LoadResource(asset.AssetLocation);
        }
        else // Property
        {
            IProperty* pProp = mpProperties->ChildByIDString(asset.AssetLocation);

            if (pProp->Type() == EPropertyType::Asset)
            {
                const auto* pAsset = TPropCast<CAssetProperty>(pProp);
                pRes = resourceStore->LoadResource(pAsset->ValueRef(pPropertyData), EResourceType::DynamicCollision);
            }
        }

        // Verify resource exists + is correct type
        if (pRes != nullptr && (pRes->Type() == EResourceType::DynamicCollision))
            return static_cast<CCollisionMeshGroup*>(pRes);
    }

    return nullptr;
}

const TString& CScriptTemplate::Name() const
{
    return mpProperties->Name();
}

bool CScriptTemplate::IsDirty() const
{
    return mDirty || mpProperties->IsDirty();
}

// ************ OBJECT TRACKING ************
uint32_t CScriptTemplate::NumObjects() const
{
    return mObjectList.size();
}

const std::list<CScriptObject*>& CScriptTemplate::ObjectList() const
{
    return mObjectList;
}

void CScriptTemplate::AddObject(CScriptObject *pObject)
{
    mObjectList.push_back(pObject);
}

void CScriptTemplate::RemoveObject(const CScriptObject *pObject)
{
    const auto iter = std::ranges::find(mObjectList, pObject);
    if (iter == mObjectList.cend())
        return;

    mObjectList.erase(iter);
}

void CScriptTemplate::SortObjects()
{
    // todo: make this function take layer names into account
    mObjectList.sort([](const CScriptObject *pA, const CScriptObject *pB) {
        return pA->InstanceID() < pB->InstanceID();
    });
}

template <>
const CEnumNameMap TEnumReflection<EAttachType>::skNameMap = {
    {0, "Attach"},
    {1, "Follow"},
};
template <>
const int TEnumReflection<EAttachType>::skErrorValue = -1;

template <>
const CEnumNameMap TEnumReflection<CScriptTemplate::ERotationType>::skNameMap = {
    {0, "RotationEnabled"},
    {1, "RotationDisabled"},
};
template <>
const int TEnumReflection<CScriptTemplate::ERotationType>::skErrorValue = -1;

template <>
const CEnumNameMap TEnumReflection<CScriptTemplate::EScaleType>::skNameMap = {
    {0, "ScaleEnabled"},
    {1, "ScaleDisabled"},
    {2, "ScaleVolume"},
};
template <>
const int TEnumReflection<CScriptTemplate::EScaleType>::skErrorValue = -1;

template <>
const CEnumNameMap TEnumReflection<CScriptTemplate::SEditorAsset::EAssetType>::skNameMap = {
    {0, "Model"},
    {1, "AnimParams"},
    {2, "Billboard"},
    {3, "Collision"},
};
template <>
const int TEnumReflection<CScriptTemplate::SEditorAsset::EAssetType>::skErrorValue = -1;

template <>
const CEnumNameMap TEnumReflection<CScriptTemplate::SEditorAsset::EAssetSource>::skNameMap = {
    {0, "Property"},
    {1, "File"},
};
template <>
const int TEnumReflection<CScriptTemplate::SEditorAsset::EAssetSource>::skErrorValue = -1;