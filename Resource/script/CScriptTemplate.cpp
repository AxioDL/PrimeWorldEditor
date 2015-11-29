#include "CScriptTemplate.h"
#include "CScriptObject.h"
#include "CMasterTemplate.h"
#include <iostream>
#include <string>
#include <Core/Log.h>
#include <Core/CResCache.h>
#include <Resource/CAnimSet.h>

CScriptTemplate::CScriptTemplate(CMasterTemplate *pMaster)
{
    mpMaster = pMaster;
    mVisible = true;
    mPreviewScale = 1.f;
    mVolumeShape = eNoShape;
}

CScriptTemplate::~CScriptTemplate()
{
    for (u32 iSet = 0; iSet < mPropertySets.size(); iSet++)
        delete mPropertySets[iSet].pBaseStruct;
}

CMasterTemplate* CScriptTemplate::MasterTemplate()
{
    return mpMaster;
}

EGame CScriptTemplate::Game()
{
    return mpMaster->GetGame();
}

TString CScriptTemplate::TemplateName(s32 propCount) const
{
    // Return original name if there is only one property set
    // or if caller doesn't want to distinguish between sets
    if ((NumPropertySets() == 1) || (propCount == -1))
        return mTemplateName;

    // Otherwise we return the template name with the set name appended
    for (auto it = mPropertySets.begin(); it != mPropertySets.end(); it++)
        if (it->pBaseStruct->Count() == propCount)
            return mTemplateName + " (" + it->SetName + ")";

    return mTemplateName + " (Invalid)";
}

TString CScriptTemplate::PropertySetNameByCount(s32 propCount) const
{
    for (auto it = mPropertySets.begin(); it != mPropertySets.end(); it++)
        if (it->pBaseStruct->Count() == propCount)
            return it->SetName;

    return "";
}

TString CScriptTemplate::PropertySetNameByIndex(u32 index) const
{
    if (index < NumPropertySets())
        return mPropertySets[index].SetName;
    else
        return "";
}

u32 CScriptTemplate::NumPropertySets() const
{
    return mPropertySets.size();
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

void CScriptTemplate::DebugPrintProperties(int propCount)
{
    CStructTemplate *pTemp = BaseStructByCount(propCount);
    if (pTemp) pTemp->DebugPrintProperties("");
}

// ************ PROPERTY FETCHING ************
template<typename t, EPropertyType propType>
t TFetchProperty(CPropertyStruct *pProperties, const TIDString& ID)
{
    if (ID.IsEmpty()) return nullptr;
    CPropertyBase *pProp = pProperties->PropertyByIDString(ID);

    if (pProp && (pProp->Type() == propType))
        return static_cast<t>(pProp);
    else
        return nullptr;
}

CStructTemplate* CScriptTemplate::BaseStructByCount(s32 propCount)
{
    if (mPropertySets.size() == 1) return mPropertySets[0].pBaseStruct;

    for (u32 iSet = 0; iSet < mPropertySets.size(); iSet++)
        if (mPropertySets[iSet].pBaseStruct->Count() == propCount)
            return mPropertySets[iSet].pBaseStruct;

    return nullptr;
}

CStructTemplate* CScriptTemplate::BaseStructByIndex(u32 index)
{
    if (index < NumPropertySets())
        return mPropertySets[index].pBaseStruct;
    else
        return nullptr;
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
        CPropertyBase *pProp = pObj->Properties()->PropertyByIDString(mVolumeConditionIDString);

        // Get value of the condition test property (only boolean, integral, and enum types supported)
        int v;
        switch (pProp->Type())
        {
        case eBoolProperty:
            v = (static_cast<CBoolProperty*>(pProp)->Get() ? 1 : 0);
            break;

        case eByteProperty:
            v = (int) static_cast<CByteProperty*>(pProp)->Get();
            break;

        case eShortProperty:
            v = (int) static_cast<CShortProperty*>(pProp)->Get();
            break;

        case eLongProperty:
            v = (int) static_cast<CLongProperty*>(pProp)->Get();
            break;

        case eEnumProperty: {
            CEnumProperty *pEnumCast = static_cast<CEnumProperty*>(pProp);
            CEnumTemplate *pEnumTemp = static_cast<CEnumTemplate*>(pEnumCast->Template());
            int index = static_cast<CEnumProperty*>(pProp)->Get();
            v = pEnumTemp->EnumeratorID(index);
            break;
        }
        }

        // Test and check whether any of the conditions are true
        for (auto it = mVolumeConditions.begin(); it != mVolumeConditions.end(); it++)
        {
            if (it->Value == v)
                return it->Shape;
        }

        Log::Error(TemplateName() + " instance " + TString::HexString(pObj->InstanceID(), true, true, 8) + " has unexpected volume shape value of " + TString::HexString((u32) v, true, true));
        return eInvalidShape;
    }

    else return mVolumeShape;
}

CStringProperty* CScriptTemplate::FindInstanceName(CPropertyStruct *pProperties)
{
    return TFetchProperty<CStringProperty*, eStringProperty>(pProperties, mNameIDString);
}

CVector3Property* CScriptTemplate::FindPosition(CPropertyStruct *pProperties)
{
    return TFetchProperty<CVector3Property*, eVector3Property>(pProperties, mPositionIDString);
}

CVector3Property* CScriptTemplate::FindRotation(CPropertyStruct *pProperties)
{
    return TFetchProperty<CVector3Property*, eVector3Property>(pProperties, mRotationIDString);
}

CVector3Property* CScriptTemplate::FindScale(CPropertyStruct *pProperties)
{
    return TFetchProperty<CVector3Property*, eVector3Property>(pProperties, mScaleIDString);
}

CBoolProperty* CScriptTemplate::FindActive(CPropertyStruct *pProperties)
{
    return TFetchProperty<CBoolProperty*, eBoolProperty>(pProperties, mActiveIDString);
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
            CPropertyBase *pProp = pProperties->PropertyByIDString(it->AssetLocation);

            if (pProp->Type() == eFileProperty)
            {
                CFileProperty *pFile = static_cast<CFileProperty*>(pProp);
                pRes = pFile->Get();
            }

            else if (pProp->Type() == eAnimParamsProperty)
            {
                CAnimParamsProperty *pParams = static_cast<CAnimParamsProperty*>(pProp);
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
            CPropertyBase *pProp = pProperties->PropertyByIDString(it->AssetLocation);

            if (pProp->Type() == eFileProperty)
            {
                CFileProperty *pFile = static_cast<CFileProperty*>(pProp);
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
            CPropertyBase *pProp = pProperties->PropertyByIDString(it->AssetLocation);

            if (pProp->Type() == eFileProperty)
            {
                CFileProperty *pFile = static_cast<CFileProperty*>(pProp);
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

        CPropertyBase *pProp = pProperties->PropertyByIDString(it->AssetLocation);

        if (pProp->Type() == eFileProperty)
        {
            CFileProperty *pFile = static_cast<CFileProperty*>(pProp);
            pRes = pFile->Get();
        }

        else if (pProp->Type() == eAnimParamsProperty)
        {
            CAnimParamsProperty *pParams = static_cast<CAnimParamsProperty*>(pProp);
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
