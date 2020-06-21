#include "CScriptAttachNode.h"
#include "CScriptNode.h"
#include "Core/Render/CRenderer.h"
#include "Core/Resource/Script/Property/IProperty.h"
#include <Common/Macros.h>

CScriptAttachNode::CScriptAttachNode(CScene *pScene, const SAttachment& rkAttachment, CScriptNode *pParent)
    : CSceneNode(pScene, -1, pParent)
    , mpScriptNode(pParent)
    , mAttachType(rkAttachment.AttachType)
    , mLocatorName(rkAttachment.LocatorName)
{
    CStructProperty* pBaseStruct = pParent->Template()->Properties();

    mpAttachAssetProp = pBaseStruct->ChildByIDString(rkAttachment.AttachProperty);
    mAttachAssetRef = CAssetRef(pParent->Instance()->PropertyData(), mpAttachAssetProp);
    mAttachAnimSetRef = CAnimationSetRef(pParent->Instance()->PropertyData(), mpAttachAssetProp);
    if (mpAttachAssetProp) AttachPropertyModified();

    ParentDisplayAssetChanged(mpScriptNode->DisplayAsset());
}

void CScriptAttachNode::AttachPropertyModified()
{
    if (mpAttachAssetProp)
    {
        if (mAttachAssetRef.IsValid())
            mpAttachAsset = gpResourceStore->LoadResource<CModel>(mAttachAssetRef.Get());
        else if (mAttachAnimSetRef.IsValid())
            mpAttachAsset = mAttachAnimSetRef.Get().AnimSet();

        CModel* pModel = Model();

        if (pModel && pModel->Type() == EResourceType::Model)
            mLocalAABox = pModel->AABox();
        else
            mLocalAABox = CAABox::Infinite();

        MarkTransformChanged();
    }
}

void CScriptAttachNode::ParentDisplayAssetChanged(CResource* pNewDisplayAsset)
{
    if (pNewDisplayAsset->Type() == EResourceType::AnimSet)
    {
        CSkeleton* pSkel = mpScriptNode->ActiveSkeleton();
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
        if (mpAttachAsset->Type() == EResourceType::Model)
            return static_cast<CModel*>(mpAttachAsset.RawPointer());

        else if (mpAttachAsset->Type() == EResourceType::AnimSet)
            return mAttachAnimSetRef.Get().GetCurrentModel();
    }

    return nullptr;
}

void CScriptAttachNode::AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo)
{
    CModel *pModel = Model();
    if (!pModel) return;

    if (rkViewInfo.ViewFrustum.BoxInFrustum(AABox()))
    {
        AddModelToRenderer(pRenderer, pModel, 0);

        if (mpParent->IsSelected() && !rkViewInfo.GameMode)
            pRenderer->AddMesh(this, -1, AABox(), false, ERenderCommand::DrawSelection);
    }
}

void CScriptAttachNode::Draw(FRenderOptions Options, int /*ComponentIndex*/, ERenderCommand Command, const SViewInfo& rkViewInfo)
{
    LoadModelMatrix();
    mpParent->LoadLights(rkViewInfo);

    CGraphics::SetupAmbientColor();
    CGraphics::UpdateVertexBlock();

    CGraphics::sPixelBlock.TintColor = mpParent->TintColor(rkViewInfo);
    CGraphics::sPixelBlock.SetAllTevColors(CColor::White());
    CGraphics::UpdatePixelBlock();
    DrawModelParts(Model(), Options, 0, Command);
}

void CScriptAttachNode::DrawSelection()
{
    LoadModelMatrix();
    glBlendFunc(GL_ONE, GL_ZERO);
    Model()->DrawWireframe(ERenderOption::None, mpParent->WireframeColor());
}

void CScriptAttachNode::RayAABoxIntersectTest(CRayCollisionTester& rTester, const SViewInfo& /*rkViewInfo*/)
{
    CModel *pModel = Model();
    if (!pModel) return;

    const CRay& rkRay = rTester.Ray();
    const std::pair<bool, float> BoxResult = AABox().IntersectsRay(rkRay);

    if (BoxResult.first)
        rTester.AddNodeModel(this, pModel);
}

SRayIntersection CScriptAttachNode::RayNodeIntersectTest(const CRay& rkRay, uint32 AssetID, const SViewInfo& rkViewInfo)
{
    const FRenderOptions Options = rkViewInfo.pRenderer->RenderOptions();

    SRayIntersection Out;
    Out.pNode = mpParent;
    Out.ComponentIndex = AssetID;

    const CRay TransformedRay = rkRay.Transformed(Transform().Inverse());
    const auto [intersects, distance] = Model()->GetSurface(AssetID)->IntersectsRay(TransformedRay, Options.HasFlag(ERenderOption::EnableBackfaceCull));

    if (intersects)
    {
        Out.Hit = true;
        const CVector3f HitPoint = TransformedRay.PointOnRay(distance);
        const CVector3f WorldHitPoint = Transform() * HitPoint;
        Out.Distance = rkRay.Origin().Distance(WorldHitPoint);
    }
    else
    {
        Out.Hit = false;
    }

    return Out;
}

// ************ PROTECTED ************
void CScriptAttachNode::CalculateTransform(CTransform4f& rOut) const
{
    // Apply our local transform
    rOut.Scale(LocalScale());
    rOut.Rotate(LocalRotation());
    rOut.Translate(LocalPosition());

    // Apply bone transform
    if (mpLocator)
        rOut = mpScriptNode->BoneTransform(mpLocator->ID(), mAttachType, false) * rOut;

    // Apply parent transform
    if (mpParent)
        rOut = mpParent->Transform() * rOut;
}
