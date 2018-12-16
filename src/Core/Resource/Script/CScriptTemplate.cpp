#include "CScriptTemplate.h"
#include "CScriptObject.h"
#include "CGameTemplate.h"
#include "Core/GameProject/CResourceStore.h"
#include "Core/Resource/Animation/CAnimSet.h"
#include <Common/Log.h>

#include <iostream>
#include <string>

// Old constructor
CScriptTemplate::CScriptTemplate(CGameTemplate *pGame)
    : mpGame(pGame)
    , mpProperties(nullptr)
    , mVisible(true)
    , mDirty(false)
    , mpNameProperty(nullptr)
    , mpPositionProperty(nullptr)
    , mpRotationProperty(nullptr)
    , mpScaleProperty(nullptr)
    , mpActiveProperty(nullptr)
    , mpLightParametersProperty(nullptr)
    , mPreviewScale(1.f)
    , mVolumeShape(EVolumeShape::NoShape)
    , mVolumeScale(1.f)
{
}

// New constructor
CScriptTemplate::CScriptTemplate(CGameTemplate* pInGame, uint32 InObjectID, const TString& kInFilePath)
    : mRotationType(ERotationType::RotationEnabled)
    , mScaleType(EScaleType::ScaleEnabled)
    , mPreviewScale(1.f)
    , mVolumeShape(EVolumeShape::NoShape)
    , mVolumeScale(1.f)
    , mSourceFile(kInFilePath)
    , mObjectID(InObjectID)
    , mpGame(pInGame)
    , mpNameProperty(nullptr)
    , mpPositionProperty(nullptr)
    , mpRotationProperty(nullptr)
    , mpScaleProperty(nullptr)
    , mpActiveProperty(nullptr)
    , mpLightParametersProperty(nullptr)
    , mVisible(true)
    , mDirty(false)
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

CScriptTemplate::~CScriptTemplate()
{
}

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

// ************ PROPERTY FETCHING ************
template<class PropType>
PropType* TFetchProperty(CStructProperty* pProperties, const TIDString& rkID)
{
    if (rkID.IsEmpty()) return nullptr;
    IProperty *pProp = pProperties->ChildByIDString(rkID);

    if (pProp && (pProp->Type() == PropEnum))
        return static_cast<PropType*>(pProp)->ValuePtr();
    else
        return nullptr;
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
        int32 Index = CheckVolumeConditions(pObj, true);
        if (Index == -1) return EVolumeShape::InvalidShape;
        else return mVolumeConditions[Index].Shape;
    }
    else return mVolumeShape;
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
        int32 Index = CheckVolumeConditions(pObj, false);
        if (Index == -1) return mVolumeScale;
        else return mVolumeConditions[Index].Scale;
    }
    else return mVolumeScale;
}

int32 CScriptTemplate::CheckVolumeConditions(CScriptObject *pObj, bool LogErrors)
{
    // Private function
    if (mVolumeShape == EVolumeShape::ConditionalShape)
    {
        TIDString PropID = mVolumeConditionIDString;
        IProperty* pProp = pObj->Template()->Properties()->ChildByIDString( PropID );

        // Get value of the condition test property (only boolean, integral, and enum types supported)
        void* pData = pObj->PropertyData();
        int Val;

        switch (pProp->Type())
        {
        case EPropertyType::Bool:
            Val = TPropCast<CBoolProperty>(pProp)->Value(pData) ? 1 : 0;
            break;

        case EPropertyType::Byte:
            Val = (int) TPropCast<CByteProperty>(pProp)->Value(pData);
            break;

        case EPropertyType::Short:
            Val = (int) TPropCast<CShortProperty>(pProp)->Value(pData);
            break;

        case EPropertyType::Int:
            Val = TPropCast<CIntProperty>(pProp)->Value(pData);
            break;

        case EPropertyType::Enum:
        case EPropertyType::Choice:
            Val = TPropCast<CEnumProperty>(pProp)->Value(pData);
            break;
        }

        // Test and check whether any of the conditions are true
        for (uint32 LinkIdx = 0; LinkIdx < mVolumeConditions.size(); LinkIdx++)
        {
            if (mVolumeConditions[LinkIdx].Value == Val)
                return LinkIdx;
        }

        if (LogErrors)
            errorf("%s instance %08X has unexpected volume shape value of 0x%X", *pObj->Template()->Name(), pObj->InstanceID(), Val);
    }

    return -1;
}

CResource* CScriptTemplate::FindDisplayAsset(void* pPropertyData, uint32& rOutCharIndex, uint32& rOutAnimIndex, bool& rOutIsInGame)
{
    rOutCharIndex = -1;
    rOutAnimIndex = -1;
    rOutIsInGame = false;

    for (auto it = mAssets.begin(); it != mAssets.end(); it++)
    {
        if (it->AssetType == SEditorAsset::EAssetType::Collision) continue;
        CResource *pRes = nullptr;

        // File
        if (it->AssetSource == SEditorAsset::EAssetSource::File)
            pRes = gpEditorStore->LoadResource(it->AssetLocation);

        // Property
        else
        {
            IProperty* pProp = mpProperties->ChildByIDString(it->AssetLocation);

            if (it->AssetType == SEditorAsset::EAssetType::AnimParams && pProp->Type() == EPropertyType::AnimationSet)
            {
                CAnimationSetProperty* pAnimSet = TPropCast<CAnimationSetProperty>(pProp);
                CAnimationParameters Params = pAnimSet->Value(pPropertyData);
                pRes = Params.AnimSet();

                if (pRes)
                {
                    uint32 MaxNumChars = static_cast<CAnimSet*>(pRes)->NumCharacters();
                    rOutCharIndex = (it->ForceNodeIndex >= 0 && it->ForceNodeIndex < (int32) MaxNumChars ? it->ForceNodeIndex : Params.CharacterIndex());
                    rOutAnimIndex = Params.AnimIndex();
                }
            }

            else
            {
                ASSERT(pProp->Type() == EPropertyType::Asset);
                CAssetProperty* pAsset = TPropCast<CAssetProperty>(pProp);
                CAssetID ID = pAsset->Value(pPropertyData);
                CResourceEntry *pEntry = gpResourceStore->FindEntry( ID );
                if (pEntry) pRes = pEntry->Load();
            }
        }

        // If we have a valid resource, return
        if (pRes)
        {
            rOutIsInGame = (pRes->Type() != EResourceType::Texture && it->AssetSource == SEditorAsset::EAssetSource::Property);
            return pRes;
        }
    }

    // None are valid - no display asset
    return nullptr;
}

CCollisionMeshGroup* CScriptTemplate::FindCollision(void* pPropertyData)
{
    for (auto it = mAssets.begin(); it != mAssets.end(); it++)
    {
        if (it->AssetType != SEditorAsset::EAssetType::Collision) continue;
        CResource *pRes = nullptr;

        // File
        if (it->AssetSource == SEditorAsset::EAssetSource::File)
            pRes = gpResourceStore->LoadResource(it->AssetLocation);

        // Property
        else
        {
            IProperty* pProp = mpProperties->ChildByIDString(it->AssetLocation);

            if (pProp->Type() == EPropertyType::Asset)
            {
                CAssetProperty* pAsset = TPropCast<CAssetProperty>(pProp);
                pRes = gpResourceStore->LoadResource( pAsset->Value(pPropertyData), EResourceType::DynamicCollision );
            }
        }

        // Verify resource exists + is correct type
        if (pRes && (pRes->Type() == EResourceType::DynamicCollision))
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

void CScriptTemplate::RemoveObject(CScriptObject *pObject)
{
    for (auto it = mObjectList.begin(); it != mObjectList.end(); it++)
    {
        if (*it == pObject)
        {
            mObjectList.erase(it);
            break;
        }
    }
}

void CScriptTemplate::SortObjects()
{
    // todo: make this function take layer names into account
    mObjectList.sort([](CScriptObject *pA, CScriptObject *pB) -> bool {
        return (pA->InstanceID() < pB->InstanceID());
    });
}
