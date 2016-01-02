#include "CScriptTemplate.h"
#include "CScriptObject.h"
#include "CMasterTemplate.h"
#include "Core/Resource/CResCache.h"
#include "Core/Resource/CAnimSet.h"
#include "Core/Log.h"

#include <iostream>
#include <string>

CScriptTemplate::CScriptTemplate(CMasterTemplate *pMaster)
    : mpMaster(pMaster)
    , mpBaseStruct(nullptr)
    , mVisible(true)
    , mPreviewScale(1.f)
    , mVolumeShape(eNoShape)
    , mVolumeScale(1.f)
{
}

CScriptTemplate::~CScriptTemplate()
{
    delete mpBaseStruct;
}

CMasterTemplate* CScriptTemplate::MasterTemplate()
{
    return mpMaster;
}

EGame CScriptTemplate::Game()
{
    return mpMaster->GetGame();
}

TString CScriptTemplate::TemplateName() const
{
    return mTemplateName;
}

CScriptTemplate::ERotationType CScriptTemplate::RotationType() const
{
    return mRotationType;
}

CScriptTemplate::EScaleType CScriptTemplate::ScaleType() const
{
    return mScaleType;
}

float CScriptTemplate::PreviewScale() const
{
    return mPreviewScale;
}

u32 CScriptTemplate::ObjectID() const
{
    return mObjectID;
}

void CScriptTemplate::SetVisible(bool visible)
{
    mVisible = visible;
}

bool CScriptTemplate::IsVisible() const
{
    return mVisible;
}

void CScriptTemplate::DebugPrintProperties()
{
    mpBaseStruct->DebugPrintProperties("");
}

// ************ PROPERTY FETCHING ************
template<typename t, EPropertyType propType>
t TFetchProperty(CPropertyStruct *pProperties, const TIDString& ID)
{
    if (ID.IsEmpty()) return nullptr;
    IProperty *pProp = pProperties->PropertyByIDString(ID);

    if (pProp && (pProp->Type() == propType))
        return static_cast<t>(pProp);
    else
        return nullptr;
}

CStructTemplate* CScriptTemplate::BaseStruct()
{
    return mpBaseStruct;
}

EVolumeShape CScriptTemplate::VolumeShape(CScriptObject *pObj)
{
    if (pObj->Template() != this)
    {
        Log::Error(pObj->Template()->TemplateName() + " instance somehow called VolumeShape() on " + TemplateName() + " template");
        return eInvalidShape;
    }

    if (mVolumeShape == eConditionalShape)
    {
        s32 index = CheckVolumeConditions(pObj, true);
        if (index == -1) return eInvalidShape;
        else return mVolumeConditions[index].Shape;
    }
    else return mVolumeShape;
}

float CScriptTemplate::VolumeScale(CScriptObject *pObj)
{
    if (pObj->Template() != this)
    {
        Log::Error(pObj->Template()->TemplateName() + " instance somehow called VolumeScale() on " + TemplateName() + " template");
        return -1;
    }

    if (mVolumeShape == eConditionalShape)
    {
        s32 index = CheckVolumeConditions(pObj, false);
        if (index == -1) return mVolumeScale;
        else return mVolumeConditions[index].Scale;
    }
    else return mVolumeScale;
}

s32 CScriptTemplate::CheckVolumeConditions(CScriptObject *pObj, bool LogErrors)
{
    // Private function
    if (mVolumeShape == eConditionalShape)
    {
        IProperty *pProp = pObj->Properties()->PropertyByIDString(mVolumeConditionIDString);

        // Get value of the condition test property (only boolean, integral, and enum types supported)
        int v;
        switch (pProp->Type())
        {
        case eBoolProperty:
            v = (static_cast<TBoolProperty*>(pProp)->Get() ? 1 : 0);
            break;

        case eByteProperty:
            v = (int) static_cast<TByteProperty*>(pProp)->Get();
            break;

        case eShortProperty:
            v = (int) static_cast<TShortProperty*>(pProp)->Get();
            break;

        case eLongProperty:
            v = (int) static_cast<TLongProperty*>(pProp)->Get();
            break;

        case eEnumProperty: {
            TEnumProperty *pEnumCast = static_cast<TEnumProperty*>(pProp);
            CEnumTemplate *pEnumTemp = static_cast<CEnumTemplate*>(pEnumCast->Template());
            int index = static_cast<TEnumProperty*>(pProp)->Get();
            v = pEnumTemp->EnumeratorID(index);
            break;
        }
        }

        // Test and check whether any of the conditions are true
        for (u32 iCon = 0; iCon < mVolumeConditions.size(); iCon++)
        {
            if (mVolumeConditions[iCon].Value == v)
                return iCon;
        }

        if (LogErrors)
            Log::Error(pObj->Template()->TemplateName() + " instance " + TString::HexString(pObj->InstanceID(), true, true, 8) + " has unexpected volume shape value of " + TString::HexString((u32) v, true, true));
    }

    return -1;
}

TStringProperty* CScriptTemplate::FindInstanceName(CPropertyStruct *pProperties)
{
    return TFetchProperty<TStringProperty*, eStringProperty>(pProperties, mNameIDString);
}

TVector3Property* CScriptTemplate::FindPosition(CPropertyStruct *pProperties)
{
    return TFetchProperty<TVector3Property*, eVector3Property>(pProperties, mPositionIDString);
}

TVector3Property* CScriptTemplate::FindRotation(CPropertyStruct *pProperties)
{
    return TFetchProperty<TVector3Property*, eVector3Property>(pProperties, mRotationIDString);
}

TVector3Property* CScriptTemplate::FindScale(CPropertyStruct *pProperties)
{
    return TFetchProperty<TVector3Property*, eVector3Property>(pProperties, mScaleIDString);
}

TBoolProperty* CScriptTemplate::FindActive(CPropertyStruct *pProperties)
{
    return TFetchProperty<TBoolProperty*, eBoolProperty>(pProperties, mActiveIDString);
}

CPropertyStruct* CScriptTemplate::FindLightParameters(CPropertyStruct *pProperties)
{
    return TFetchProperty<CPropertyStruct*, eStructProperty>(pProperties, mLightParametersIDString);
}

// todo: merge these four functions, they have near-identical code
CModel* CScriptTemplate::FindDisplayModel(CPropertyStruct *pProperties)
{
    for (auto it = mAssets.begin(); it != mAssets.end(); it++)
    {
        if ((it->AssetType != SEditorAsset::eModel) && (it->AssetType != SEditorAsset::eAnimParams)) continue;
        CResource *pRes = nullptr;

        // File
        if (it->AssetSource == SEditorAsset::eFile)
        {
            TString path = "../resources/" + it->AssetLocation;
            pRes = gResCache.GetResource(path);
        }

        // Property
        else
        {
            IProperty *pProp = pProperties->PropertyByIDString(it->AssetLocation);

            if (pProp->Type() == eFileProperty)
            {
                TFileProperty *pFile = static_cast<TFileProperty*>(pProp);
                pRes = pFile->Get();
            }

            else if (pProp->Type() == eCharacterProperty)
            {
                TAnimParamsProperty *pParams = static_cast<TAnimParamsProperty*>(pProp);
                pRes = pParams->Get().GetCurrentModel(it->ForceNodeIndex);
            }
        }

        // Verify resource exists + is correct type
        if (pRes && (pRes->Type() == eModel))
            return static_cast<CModel*>(pRes);
    }

    return nullptr;
}

CTexture* CScriptTemplate::FindBillboardTexture(CPropertyStruct *pProperties)
{
    for (auto it = mAssets.begin(); it != mAssets.end(); it++)
    {
        if (it->AssetType != SEditorAsset::eBillboard) continue;
        CResource *pRes = nullptr;

        // File
        if (it->AssetSource == SEditorAsset::eFile)
        {
            TString path = "../resources/" + it->AssetLocation;
            pRes = gResCache.GetResource(path);
        }

        // Property
        else
        {
            IProperty *pProp = pProperties->PropertyByIDString(it->AssetLocation);

            if (pProp->Type() == eFileProperty)
            {
                TFileProperty *pFile = static_cast<TFileProperty*>(pProp);
                pRes = pFile->Get();
            }
        }

        // Verify resource exists + is correct type
        if (pRes && (pRes->Type() == eTexture))
            return static_cast<CTexture*>(pRes);
    }

    return nullptr;
}

CCollisionMeshGroup* CScriptTemplate::FindCollision(CPropertyStruct *pProperties)
{
    for (auto it = mAssets.begin(); it != mAssets.end(); it++)
    {
        if (it->AssetType != SEditorAsset::eCollision) continue;
        CResource *pRes = nullptr;

        // File
        if (it->AssetSource == SEditorAsset::eFile)
        {
            TString path = "../resources/" + it->AssetLocation;
            pRes = gResCache.GetResource(path);
        }

        // Property
        else
        {
            IProperty *pProp = pProperties->PropertyByIDString(it->AssetLocation);

            if (pProp->Type() == eFileProperty)
            {
                TFileProperty *pFile = static_cast<TFileProperty*>(pProp);
                pRes = pFile->Get();
            }
        }

        // Verify resource exists + is correct type
        if (pRes && (pRes->Type() == eCollisionMeshGroup))
            return static_cast<CCollisionMeshGroup*>(pRes);
    }

    return nullptr;
}

bool CScriptTemplate::HasInGameModel(CPropertyStruct *pProperties)
{
    for (auto it = mAssets.begin(); it != mAssets.end(); it++)
    {
        if ((it->AssetType != SEditorAsset::eModel) && (it->AssetType != SEditorAsset::eAnimParams)) continue;
        if (it->AssetSource == SEditorAsset::eFile) continue;
        CResource *pRes = nullptr;

        IProperty *pProp = pProperties->PropertyByIDString(it->AssetLocation);

        if (pProp->Type() == eFileProperty)
        {
            TFileProperty *pFile = static_cast<TFileProperty*>(pProp);
            pRes = pFile->Get();
        }

        else if (pProp->Type() == eCharacterProperty)
        {
            TAnimParamsProperty *pParams = static_cast<TAnimParamsProperty*>(pProp);
            pRes = pParams->Get().GetCurrentModel(it->ForceNodeIndex);
        }

        // Verify resource exists + is correct type
        if (pRes && (pRes->Type() == eModel))
            return true;
    }

    return false;
}

bool CScriptTemplate::HasPosition()
{
    return (!mPositionIDString.IsEmpty());
}

// ************ OBJECT TRACKING ************
u32 CScriptTemplate::NumObjects() const
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
