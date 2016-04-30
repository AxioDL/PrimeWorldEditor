#include "CScriptAttachNode.h"
#include "CScriptNode.h"
#include "Core/Render/CRenderer.h"
#include "Core/Resource/Script/IProperty.h"
#include <Common/Assert.h>

CScriptAttachNode::CScriptAttachNode(CScene *pScene, const TIDString& rkAttachProperty, const TString& rkLocator, CScriptNode *pParent)
    : CSceneNode(pScene, -1, pParent)
    , mpScriptNode(pParent)
    , mLocatorName(rkLocator)
{
    CPropertyStruct *pBaseStruct = pParent->Instance()->Properties();
    mpAttachAssetProp = pBaseStruct->PropertyByIDString(rkAttachProperty);
    if (mpAttachAssetProp) AttachPropertyModified();

    ParentDisplayAssetChanged(mpScriptNode->DisplayAsset());
}

void CScriptAttachNode::AttachPropertyModified()
{
    if (mpAttachAssetProp)
    {
        if (mpAttachAssetProp->Type() == eFileProperty)
            mpAttachAsset = TPropCast<TFileProperty>(mpAttachAssetProp)->Get().Load();
        else if (mpAttachAssetProp->Type() == eCharacterProperty)
            mpAttachAsset = TPropCast<TCharacterProperty>(mpAttachAssetProp)->Get().AnimSet();

        CModel *pModel = Model();

        if (pModel && pModel->Type() == eModel)
            mLocalAABox = pModel->AABox();
        else
            mLocalAABox = CAABox::skInfinite;

        MarkTransformChanged();
    }
}

void CScriptAttachNode::ParentDisplayAssetChanged(CResource *pNewDisplayAsset)
{
    if (pNewDisplayAsset->Type() == eAnimSet)
    {
        CSkeleton *pSkel = mpScriptNode->ActiveSkeleton();
        mpLocator = pSkel->BoneByName(mLocatorName);
    }

    else
    {
        mpLocator = nullptr;
    }

    MarkTransformChanged();
}

CModel* CScriptAttachNode::Model() const
{
    if (mpAttachAsset)
    {
        if (mpAttachAsset->Type() == eModel)
            return static_cast<CModel*>(mpAttachAsset.RawPointer());

        if (mpAttachAsset->Type() == eAnimSet)
        {
            TCharacterProperty *pProp = TPropCast<TCharacterProperty>(mpAttachAssetProp);
            return pProp->Get().GetCurrentModel();
        }
    }

    return nullptr;
}

void CScriptAttachNode::AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo)
{
    CModel *pModel = Model();
    if (!pModel) return;

    if (rkViewInfo.ViewFrustum.BoxInFrustum(AABox()))
    {
        if (pModel->HasTransparency(0))
            AddSurfacesToRenderer(pRenderer, pModel, 0, rkViewInfo);
        else
            pRenderer->AddMesh(this, -1, AABox(), false, eDrawMesh);

        if (mpParent->IsSelected() && !rkViewInfo.GameMode)
            pRenderer->AddMesh(this, -1, AABox(), false, eDrawSelection);
    }
}

void CScriptAttachNode::Draw(FRenderOptions Options, int ComponentIndex, const SViewInfo& rkViewInfo)
{
    LoadModelMatrix();
    mpParent->LoadLights(rkViewInfo);

    CGraphics::SetupAmbientColor();
    CGraphics::UpdateVertexBlock();

    CGraphics::sPixelBlock.TintColor = mpParent->TintColor(rkViewInfo);
    CGraphics::sPixelBlock.TevColor = CColor::skWhite;
    CGraphics::UpdatePixelBlock();

    if (ComponentIndex < 0)
        Model()->Draw(Options, 0);
    else
        Model()->DrawSurface(Options, ComponentIndex, 0);
}

void CScriptAttachNode::DrawSelection()
{
    LoadModelMatrix();
    glBlendFunc(GL_ONE, GL_ZERO);
    Model()->DrawWireframe(eNoRenderOptions, mpParent->WireframeColor());
}

void CScriptAttachNode::RayAABoxIntersectTest(CRayCollisionTester& rTester, const SViewInfo& /*rkViewInfo*/)
{
    CModel *pModel = Model();
    if (!pModel) return;

    const CRay& rkRay = rTester.Ray();
    std::pair<bool,float> BoxResult = AABox().IntersectsRay(rkRay);

    if (BoxResult.first)
        rTester.AddNodeModel(this, pModel);
}

SRayIntersection CScriptAttachNode::RayNodeIntersectTest(const CRay& rkRay, u32 AssetID, const SViewInfo& rkViewInfo)
{
    FRenderOptions Options = rkViewInfo.pRenderer->RenderOptions();

    SRayIntersection Out;
    Out.pNode = mpParent;
    Out.ComponentIndex = AssetID;

    CRay TransformedRay = rkRay.Transformed(Transform().Inverse());
    std::pair<bool,float> Result = Model()->GetSurface(AssetID)->IntersectsRay(TransformedRay, Options.HasFlag(eEnableBackfaceCull));

    if (Result.first)
    {
        Out.Hit = true;
        CVector3f HitPoint = TransformedRay.PointOnRay(Result.second);
        CVector3f WorldHitPoint = Transform() * HitPoint;
        Out.Distance = rkRay.Origin().Distance(WorldHitPoint);
    }

    else Out.Hit = false;

    return Out;
}

// ************ PROTECTED ************
void CScriptAttachNode::CalculateTransform(CTransform4f& rOut) const
{
    if (mpLocator)
        rOut = mpScriptNode->BoneTransform(mpLocator->ID(), false);

    CSceneNode::CalculateTransform(rOut);
}
