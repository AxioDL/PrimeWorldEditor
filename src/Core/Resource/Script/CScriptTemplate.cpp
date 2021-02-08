#include "CScriptTemplate.h"
#include "CScriptObject.h"
#include "CGameTemplate.h"
#include "Core/GameProject/CResourceStore.h"
#include "Core/Resource/Animation/CAnimSet.h"
#include <Common/Log.h>

#include <algorithm>
#include <string>

// Old constructor
CScriptTemplate::CScriptTemplate(CGameTemplate *pGame)
    : mpGame(pGame)
{
}

// New constructor
CScriptTemplate::CScriptTemplate(CGameTemplate* pInGame, uint32 InObjectID, const TString& kInFilePath)
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
        debugf("Saving script template: %s", *mSourceFile);
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
        errorf("%s instance somehow called VolumeShape() on %s template", *pObj->Template()->Name(), *Name());
        return EVolumeShape::InvalidShape;
    }

    if (mVolumeShape == EVolumeShape::ConditionalShape)
    {
        const int32 Index = CheckVolumeConditions(pObj, true);
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
        errorf("%s instance somehow called VolumeScale() on %s template", *pObj->Template()->Name(), *Name());
        return -1;
    }

    if (mVolumeShape == EVolumeShape::ConditionalShape)
    {
        const int32 Index = CheckVolumeConditions(pObj, false);
        if (Index == -1)
            return mVolumeScale;

        return mVolumeConditions[Index].Scale;
    }

    return mVolumeScale;
}

int32 CScriptTemplate::CheckVolumeConditions(CScriptObject *pObj, bool LogErrors)
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
        for (uint32 LinkIdx = 0; LinkIdx < mVolumeConditions.size(); LinkIdx++)
        {
            if (mVolumeConditions[LinkIdx].Value == Val)
                return LinkIdx;
        }

        if (LogErrors)
        {
            errorf("%s instance %08X has unexpected volume shape value of 0x%X", *pObj->Template()->Name(),
                   static_cast<uint32>(pObj->InstanceID()), Val);
        }
    }

    return -1;
}

CResource* CScriptTemplate::FindDisplayAsset(void* pPropertyData, uint32& rOutCharIndex, uint32& rOutAnimIndex, bool& rOutIsInGame)
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
                auto* pAnimSet = TPropCast<CAnimationSetProperty>(pProp);
                const CAnimationParameters Params = pAnimSet->Value(pPropertyData);
                pRes = Params.AnimSet();

                if (pRes != nullptr)
                {
                    const size_t MaxNumChars = static_cast<const CAnimSet*>(pRes)->NumCharacters();
                    rOutCharIndex = (asset.ForceNodeIndex >= 0 && asset.ForceNodeIndex < static_cast<int32>(MaxNumChars) ? asset.ForceNodeIndex : Params.CharacterIndex());
                    rOutAnimIndex = Params.AnimIndex();
                }
            }

            else
            {
                ASSERT(pProp->Type() == EPropertyType::Asset);
                auto* pAsset = TPropCast<CAssetProperty>(pProp);
                const CAssetID ID = pAsset->Value(pPropertyData);
                if (CResourceEntry* pEntry = gpResourceStore->FindEntry(ID))
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

CCollisionMeshGroup* CScriptTemplate::FindCollision(void* pPropertyData)
{
    for (const auto& asset : mAssets)
    {
        if (asset.AssetType != SEditorAsset::EAssetType::Collision)
            continue;

        CResource *pRes = nullptr;

        // File
        if (asset.AssetSource == SEditorAsset::EAssetSource::File)
        {
            pRes = gpResourceStore->LoadResource(asset.AssetLocation);
        }
        else // Property
        {
            IProperty* pProp = mpProperties->ChildByIDString(asset.AssetLocation);

            if (pProp->Type() == EPropertyType::Asset)
            {
                auto* pAsset = TPropCast<CAssetProperty>(pProp);
                pRes = gpResourceStore->LoadResource( pAsset->Value(pPropertyData), EResourceType::DynamicCollision );
            }
        }

        // Verify resource exists + is correct type
        if (pRes != nullptr && (pRes->Type() == EResourceType::DynamicCollision))
            return static_cast<CCollisionMeshGroup*>(pRes);
    }

    return nullptr;
}


// ************ OBJECT TRACKING ************
uint32 CScriptTemplate::NumObjects() const
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
    const auto iter = std::find_if(mObjectList.cbegin(), mObjectList.cend(),
                                   [pObject](const auto* ptr) { return ptr == pObject; });

    if (iter == mObjectList.cend())
        return;

    mObjectList.erase(iter);
}

void CScriptTemplate::SortObjects()
{
    // todo: make this function take layer names into account
    mObjectList.sort([](CScriptObject *pA, CScriptObject *pB) -> bool {
        return (pA->InstanceID() < pB->InstanceID());
    });
}
