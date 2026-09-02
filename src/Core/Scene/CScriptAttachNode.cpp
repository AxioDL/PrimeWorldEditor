#include "Core/Scene/CScriptAttachNode.h"

#include "Core/SRayIntersection.h"
#include "Core/Render/CGraphics.h"
#include "Core/Render/CRenderer.h"
#include "Core/Resource/Animation/CSkeleton.h"
#include "Core/Resource/Model/SSurface.h"
#include "Core/Resource/Script/CScriptObject.h"
#include "Core/Resource/Script/Property/IProperty.h"
#include "Core/Scene/CScene.h"
#include "Core/Scene/CScriptNode.h"
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
    if (mpAttachAssetProp)
        AttachPropertyModified();

    ParentDisplayAssetChanged(mpScriptNode->DisplayAsset());
}

CScriptAttachNode::~CScriptAttachNode() = default;

void CScriptAttachNode::AttachPropertyModified()
{
    if (!mpAttachAssetProp)
        return;

    if (mAttachAssetRef.IsValid())
        mpAttachAsset = Scene()->ActiveArea()->Entry()->ResourceStore()->LoadResource<CModel>(mAttachAssetRef.Get());
    else if (mAttachAnimSetRef.IsValid())
        mpAttachAsset = mAttachAnimSetRef.Get().AnimSet();

    const CModel* pModel = Model();
    if (pModel && pModel->Type() == EResourceType::Model)
        mLocalAABox = pModel->AABox();
    else
        mLocalAABox = CAABox::Infinite();

    MarkTransformChanged();
}

void CScriptAttachNode::ParentDisplayAssetChanged(const CResource* pNewDisplayAsset)
{
    if (pNewDisplayAsset && pNewDisplayAsset->Type() == EResourceType::AnimSet)
    {
        const CSkeleton* pSkel = mpScriptNode->ActiveSkeleton();
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

        if (mpAttachAsset->Type() == EResourceType::AnimSet)
            return mAttachAnimSetRef.Get().GetCurrentModel();
    }

    return nullptr;
}

void CScriptAttachNode::AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo)
{
    CModel *pModel = Model();
    if (!pModel)
        return;

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

    if (Command == ERenderCommand::DrawSelection)
    {
        glBlendFunc(GL_ONE, GL_ZERO);
        Model()->DrawWireframe(ERenderOption::None, mpParent->WireframeColor());
        return;
    }

    mpParent->LoadLights(rkViewInfo);

    CGraphics::SetupAmbientColor();
    CGraphics::UpdateVertexBlock();

    CGraphics::sPixelBlock.TintColor = mpParent->TintColor(rkViewInfo);
    CGraphics::sPixelBlock.SetAllTevColors(CColor::White());
    CGraphics::UpdatePixelBlock();
    DrawModelParts(Model(), Options, 0, Command);
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

SRayIntersection CScriptAttachNode::RayNodeIntersectTest(const CRay& rkRay, uint32_t AssetID, const SViewInfo& rkViewInfo)
{
    const FRenderOptions Options = rkViewInfo.pRenderer->RenderOptions();

    SRayIntersection Out;
    Out.pNode = mpParent;
    Out.ComponentIndex = AssetID;

    const CRay TransformedRay = rkRay.Transformed(Transform().Inverse());
    const auto [intersects, distance] = Model()->GetSurface(AssetID)->IntersectsRay(TransformedRay, !Options.HasFlag(ERenderOption::EnableBackfaceCull));

    if (intersects)
    {
        Out.Hit = true;
        const CVector3f HitPoint = TransformedRay.PointOnRay(distance);
        const CVector3f WorldHitPoint = Transform() * HitPoint;
        Out.Distance = rkRay.Origin().Distance(WorldHitPoint);
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
