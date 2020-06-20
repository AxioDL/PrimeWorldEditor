#include "CScriptNode.h"
#include "CScene.h"
#include "Core/GameProject/CResourceStore.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CGraphics.h"
#include "Core/Render/CRenderer.h"
#include "Core/Resource/Animation/CAnimSet.h"
#include "Core/Resource/Script/CGameTemplate.h"
#include "Core/Resource/Script/CScriptLayer.h"
#include "Core/ScriptExtra/CScriptExtra.h"
#include <Common/Macros.h>
#include <Common/Math/MathUtil.h>

CScriptNode::CScriptNode(CScene *pScene, uint32 NodeID, CSceneNode *pParent, CScriptObject *pInstance)
    : CSceneNode(pScene, NodeID, pParent)
    , mpInstance(pInstance)
{
    ASSERT(pInstance);

    // Evaluate instance
    SetDisplayAsset(nullptr);
    mpCollisionNode = new CCollisionNode(pScene, -1, this);
    mpCollisionNode->SetInheritance(true, true, false);

    if (mpInstance != nullptr)
    {
        const CScriptTemplate* pTemp = Template();

        // Determine transform
        mHasValidPosition = pTemp->PositionProperty() != nullptr;
        mPosition = mpInstance->Position();
        mRotation = CQuaternion::FromEuler(mpInstance->Rotation());
        mScale = mpInstance->Scale();
        MarkTransformChanged();

        SetName("[" + pTemp->Name() + "] " + mpInstance->InstanceName());

        // Determine display assets
        SetDisplayAsset(mpInstance->DisplayAsset());
        mpCollisionNode->SetCollision(mpInstance->Collision());

        // Create preview volume node
        mpVolumePreviewNode = new CModelNode(pScene, -1, this, nullptr);

        if (pTemp->ScaleType() == CScriptTemplate::EScaleType::ScaleVolume)
        {
            UpdatePreviewVolume();

            if (mHasVolumePreview)
            {
                mpVolumePreviewNode->SetInheritance(true, (mpInstance->VolumeShape() != EVolumeShape::AxisAlignedBoxShape), true);
                mpVolumePreviewNode->ForceAlphaEnabled(true);
            }
        }

        // Create attachment nodes
        for (size_t iAttach = 0; iAttach < pTemp->NumAttachments(); iAttach++)
        {
            const SAttachment& rkAttach = pTemp->Attachment(iAttach);
            mAttachments.push_back(new CScriptAttachNode(pScene, rkAttach, this));
        }

        // Fetch LightParameters
        mpLightParameters = std::make_unique<CLightParameters>(mpInstance->LightParameters(), mpInstance->GameTemplate()->Game());
        SetLightLayerIndex(mpLightParameters->LightLayerIndex());
    }
    else
    {
        // Shouldn't ever happen
        SetName("ScriptNode - NO INSTANCE");
    }

    mpExtra = CScriptExtra::CreateExtra(this);
}

ENodeType CScriptNode::NodeType()
{
    return ENodeType::Script;
}

void CScriptNode::PostLoad()
{
    CModel* pModel = ActiveModel();

    if (pModel == nullptr)
        return;

    pModel->BufferGL();
    pModel->GenerateMaterialShaders();
}

void CScriptNode::OnTransformed()
{
    if (mpInstance != nullptr)
    {
        if (LocalPosition() != mpInstance->Position())
            mpInstance->SetPosition(LocalPosition());

        if (LocalRotation().ToEuler() != mpInstance->Rotation())
            mpInstance->SetRotation(LocalRotation().ToEuler());

        if (LocalScale() != mpInstance->Scale())
            mpInstance->SetScale(LocalScale());
    }

    if (mpExtra != nullptr)
        mpExtra->OnTransformed();
}

void CScriptNode::AddToRenderer(CRenderer* pRenderer, const SViewInfo& rkViewInfo)
{
    if (mpInstance == nullptr)
        return;

    // Add script extra to renderer first
    if (mpExtra != nullptr)
        mpExtra->AddToRenderer(pRenderer, rkViewInfo);

    // If we're in game mode, then override other visibility settings.
    if (rkViewInfo.GameMode)
    {
        if (mGameModeVisibility == EGameModeVisibility::Untested)
            TestGameModeVisibility();

        if (mGameModeVisibility != EGameModeVisibility::Visible)
            return;
    }

    // Check whether the script extra wants us to render before we render.
    const bool ShouldDraw = mpExtra == nullptr || mpExtra->ShouldDrawNormalAssets();

    if (ShouldDraw)
    {
        // Otherwise, we proceed as normal
        if ((rkViewInfo.ShowFlags & EShowFlag::ObjectCollision) != 0 && !rkViewInfo.GameMode)
            mpCollisionNode->AddToRenderer(pRenderer, rkViewInfo);

        if ((rkViewInfo.ShowFlags & EShowFlag::ObjectGeometry) != 0 || rkViewInfo.GameMode)
        {
            for (auto& attachment : mAttachments)
                attachment->AddToRenderer(pRenderer, rkViewInfo);

            if (rkViewInfo.ViewFrustum.BoxInFrustum(AABox()))
            {
                if (CModel* pModel = ActiveModel())
                    AddModelToRenderer(pRenderer, pModel, 0);
                else
                    pRenderer->AddMesh(this, -1, AABox(), false, ERenderCommand::DrawMesh);
            }
        }
    }

    if (IsSelected() && !rkViewInfo.GameMode)
    {
        // Script nodes always draw their selections regardless of frustum planes
        // in order to ensure that script connection lines don't get improperly culled.
       if (ShouldDraw)
           pRenderer->AddMesh(this, -1, AABox(), false, ERenderCommand::DrawSelection);

        if (mHasVolumePreview && (mpExtra == nullptr || mpExtra->ShouldDrawVolume()))
            mpVolumePreviewNode->AddToRenderer(pRenderer, rkViewInfo);
    }
}

void CScriptNode::Draw(FRenderOptions Options, int /*ComponentIndex*/, ERenderCommand Command, const SViewInfo& rkViewInfo)
{
    if (mpInstance == nullptr)
        return;

    // Draw model
    if (UsesModel())
    {
        const auto LightingOptions = (mpLightParameters ? mpLightParameters->WorldLightingOptions() : EWorldLightingOptions::NormalLighting);

        if (CGraphics::sLightMode == CGraphics::ELightingMode::World && LightingOptions == EWorldLightingOptions::DisableWorldLighting)
        {
            CGraphics::sNumLights = 0;
            CGraphics::sVertexBlock.COLOR0_Amb = CColor::TransparentBlack();
            CGraphics::sVertexBlock.COLOR0_Mat = CColor::TransparentWhite();
            CGraphics::sPixelBlock.LightmapMultiplier = 1.f;
            CGraphics::UpdateLightBlock();
        }
        else
        {
            // DKCR doesn't support world lighting yet, so light nodes that don't have ingame models with default lighting
            if (Template()->Game() == EGame::DKCReturns && !mpInstance->HasInGameModel() && CGraphics::sLightMode == CGraphics::ELightingMode::World)
            {
                CGraphics::SetDefaultLighting();
                CGraphics::sVertexBlock.COLOR0_Amb = CGraphics::skDefaultAmbientColor;
                CGraphics::sVertexBlock.COLOR0_Mat = CColor::TransparentWhite();
            }
            else
            {
                LoadLights(rkViewInfo);
            }
        }

        LoadModelMatrix();

        // Draw model if possible!
        if (CModel* pModel = ActiveModel())
        {
            if (pModel->IsSkinned())
                CGraphics::LoadIdentityBoneTransforms();

            if (mpExtra != nullptr)
                CGraphics::sPixelBlock.SetAllTevColors(mpExtra->TevColor());
            else
                CGraphics::sPixelBlock.SetAllTevColors(CColor::White());

            CGraphics::sPixelBlock.TintColor = TintColor(rkViewInfo);
            CGraphics::UpdatePixelBlock();
            DrawModelParts(pModel, Options, 0, Command);
        }
        else // If no model or billboard, default to drawing a purple box
        {
            glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ZERO, GL_ZERO);
            glDepthMask(GL_TRUE);
            CGraphics::UpdateVertexBlock();
            CGraphics::UpdatePixelBlock();
            CDrawUtil::DrawShadedCube(CColor::TransparentPurple() * TintColor(rkViewInfo));
        }
    }
    // Draw billboard
    else if (mpDisplayAsset->Type() == EResourceType::Texture)
    {
        CDrawUtil::DrawBillboard(ActiveBillboard(), mPosition, BillboardScale(), TintColor(rkViewInfo));
    }
}

void CScriptNode::DrawSelection()
{
    glBlendFunc(GL_ONE, GL_ZERO);
    LoadModelMatrix();

    // Draw wireframe for models
    if (UsesModel())
    {
        CModel* pModel = ActiveModel();
        if (pModel == nullptr)
            pModel = CDrawUtil::GetCubeModel();

        pModel->DrawWireframe(ERenderOption::None, WireframeColor());
    }

    // Draw rotation arrow for billboards
    else
    {
        // Create model matrix that doesn't take scaling into account, and draw
        CTransform4f Transform;
        Transform.Rotate(AbsoluteRotation());
        Transform.Translate(AbsolutePosition());
        CGraphics::sMVPBlock.ModelMatrix = Transform;
        CGraphics::UpdateMVPBlock();

        CGraphics::sPixelBlock.TintColor = CColor::White();
        CGraphics::UpdatePixelBlock();

        DrawRotationArrow();
    }

    if (mpInstance != nullptr)
    {
        CGraphics::sMVPBlock.ModelMatrix = CMatrix4f::skIdentity;
        CGraphics::UpdateMVPBlock();

        for (size_t iIn = 0; iIn < mpInstance->NumLinks(ELinkType::Incoming); iIn++)
        {
            // Don't draw in links if the other object is selected.
            const CLink* pLink = mpInstance->Link(ELinkType::Incoming, iIn);
            const CScriptNode* pLinkNode = mpScene->NodeForInstanceID(pLink->SenderID());
            if (pLinkNode != nullptr && !pLinkNode->IsSelected())
                CDrawUtil::DrawLine(CenterPoint(), pLinkNode->CenterPoint(), CColor::TransparentRed());
        }

        for (size_t iOut = 0; iOut < mpInstance->NumLinks(ELinkType::Outgoing); iOut++)
        {
            const CLink* pLink = mpInstance->Link(ELinkType::Outgoing, iOut);
            const CScriptNode* pLinkNode = mpScene->NodeForInstanceID(pLink->ReceiverID());
            if (pLinkNode != nullptr)
                CDrawUtil::DrawLine(CenterPoint(), pLinkNode->CenterPoint(), CColor::TransparentGreen());
        }
    }
}

void CScriptNode::RayAABoxIntersectTest(CRayCollisionTester& rTester, const SViewInfo& rkViewInfo)
{
    if (mpInstance == nullptr)
        return;

    // Let script extra do ray check first
    if (mpExtra != nullptr)
    {
        mpExtra->RayAABoxIntersectTest(rTester, rkViewInfo);

        // If the extra doesn't want us rendering, then don't do the ray test either
        if (!mpExtra->ShouldDrawNormalAssets())
            return;
    }

    // If we're in game mode, then check whether we're visible before proceeding with the ray test.
    if (rkViewInfo.GameMode)
    {
        if (mGameModeVisibility == EGameModeVisibility::Untested)
            TestGameModeVisibility();

        if (mGameModeVisibility != EGameModeVisibility::Visible)
            return;
    }

    // Otherwise, proceed with the ray test as normal...
    const CRay& rkRay = rTester.Ray();

    if (UsesModel())
    {
        const auto [intersects, distance] = AABox().IntersectsRay(rkRay);

        if (intersects)
        {
            if (CModel* pModel = ActiveModel())
                rTester.AddNodeModel(this, pModel);
            else
                rTester.AddNode(this, 0, distance);
        }
    }

    else
    {
        // Because the billboard rotates a lot, expand the AABox on the X/Y axes to cover any possible orientation
        const CVector2f BillScale = BillboardScale();
        const float ScaleXY = (BillScale.X > BillScale.Y ? BillScale.X : BillScale.Y);

        const CAABox BillBox = CAABox(mPosition + CVector3f(-ScaleXY, -ScaleXY, -BillScale.Y),
                                      mPosition + CVector3f(ScaleXY, ScaleXY, BillScale.Y));

        const auto [intersects, distance] = BillBox.IntersectsRay(rkRay);
        if (intersects)
            rTester.AddNode(this, 0, distance);
    }

    // Run ray check on child nodes as well
    mpCollisionNode->RayAABoxIntersectTest(rTester, rkViewInfo);

    for (auto& attachment : mAttachments)
        attachment->RayAABoxIntersectTest(rTester, rkViewInfo);
}

SRayIntersection CScriptNode::RayNodeIntersectTest(const CRay& rkRay, uint32 AssetID, const SViewInfo& rkViewInfo)
{
    const FRenderOptions Options = rkViewInfo.pRenderer->RenderOptions();

    SRayIntersection Out;
    Out.pNode = this;
    Out.ComponentIndex = AssetID;

    // Model test
    if (UsesModel())
    {
        CModel* pModel = ActiveModel();
        if (!pModel)
            pModel = CDrawUtil::GetCubeModel();

        const CRay TransformedRay = rkRay.Transformed(Transform().Inverse());
        const auto [intersects, distance] = pModel->GetSurface(AssetID)->IntersectsRay(TransformedRay, ((Options & ERenderOption::EnableBackfaceCull) == 0));

        if (intersects)
        {
            Out.Hit = true;

            const CVector3f HitPoint = TransformedRay.PointOnRay(distance);
            const CVector3f WorldHitPoint = Transform() * HitPoint;
            Out.Distance = Math::Distance(rkRay.Origin(), WorldHitPoint);
        }
        else
        {
            Out.Hit = false;
        }
    }
    // Billboard test
    // todo: come up with a better way to share this code between CScriptNode and CLightNode
    else
    {
        // Step 1: check whether the ray intersects with the plane the billboard is on
        const CPlane BillboardPlane(-rkViewInfo.pCamera->Direction(), mPosition);
        const auto [intersects, distance] = Math::RayPlaneIntersection(rkRay, BillboardPlane);

        if (intersects)
        {
            // Step 2: transform the hit point into the plane's local space
            const CVector3f PlaneHitPoint = rkRay.PointOnRay(distance);
            const CVector3f RelHitPoint = PlaneHitPoint - mPosition;

            const CVector3f PlaneForward = -rkViewInfo.pCamera->Direction();
            const CVector3f PlaneRight = -rkViewInfo.pCamera->RightVector();
            const CVector3f PlaneUp = rkViewInfo.pCamera->UpVector();
            const CQuaternion PlaneRot = CQuaternion::FromAxes(PlaneRight, PlaneForward, PlaneUp);

            const CVector3f RotatedHitPoint = PlaneRot.Inverse() * RelHitPoint;
            const CVector2f LocalHitPoint = RotatedHitPoint.XZ() / BillboardScale();

            // Step 3: check whether the transformed hit point is in the -1 to 1 range
            if ((LocalHitPoint.X >= -1.f) && (LocalHitPoint.X <= 1.f) && (LocalHitPoint.Y >= -1.f) && (LocalHitPoint.Y <= 1.f))
            {
                // Step 4: look up the hit texel and check whether it's transparent or opaque
                CVector2f TexCoord = (LocalHitPoint + CVector2f(1.f)) * 0.5f;
                TexCoord.X = -TexCoord.X + 1.f;
                const float TexelAlpha = ActiveBillboard()->ReadTexelAlpha(TexCoord);

                if (TexelAlpha < 0.25f)
                {
                    Out.Hit = false;
                }
                else
                {
                    // It's opaque... we have a hit!
                    Out.Hit = true;
                    Out.Distance = distance;
                }
            }
            else
            {
                Out.Hit = false;
            }
        }
        else
        {
            Out.Hit = false;
        }
    }

    return Out;
}

bool CScriptNode::AllowsRotate() const
{
    return Template()->RotationType() == CScriptTemplate::ERotationType::RotationEnabled;
}

bool CScriptNode::AllowsScale() const
{
    return Template()->ScaleType() != CScriptTemplate::EScaleType::ScaleDisabled;
}

bool CScriptNode::IsVisible() const
{
    // Reimplementation of CSceneNode::IsVisible() to allow for layer and template visiblity to be taken into account
    return mVisible && mpInstance->Layer()->IsVisible() && Template()->IsVisible();
}

CColor CScriptNode::TintColor(const SViewInfo& ViewInfo) const
{
    CColor BaseColor = CSceneNode::TintColor(ViewInfo);

    if (mpExtra != nullptr)
        mpExtra->ModifyTintColor(BaseColor);

    return BaseColor;
}

void CScriptNode::LinksModified()
{
    if (mpExtra == nullptr)
        return;

    mpExtra->LinksModified();
}

CStructRef CScriptNode::GetProperties() const
{
    return CStructRef(mpInstance->PropertyData(), mpInstance->Template()->Properties());
}

void CScriptNode::PropertyModified(IProperty* pProp)
{
    // Update volume
    const EPropertyType Type = pProp->Type();

    if (Type == EPropertyType::Bool || Type == EPropertyType::Byte || Type == EPropertyType::Short ||
        Type == EPropertyType::Int || Type == EPropertyType::Choice || Type == EPropertyType::Enum)
    {
        mpInstance->EvaluateVolume();
        UpdatePreviewVolume();
    }
    // Update resources
    else if (Type == EPropertyType::AnimationSet)
    {
        mpInstance->EvaluateDisplayAsset();
        SetDisplayAsset(mpInstance->DisplayAsset());
    }
    else if (Type == EPropertyType::Asset)
    {
        const auto* pAssetProperty = TPropCast<CAssetProperty>(pProp);
        const CResTypeFilter& rkFilter = pAssetProperty->GetTypeFilter();

        if (rkFilter.Accepts(EResourceType::Model) || rkFilter.Accepts(EResourceType::Texture) ||
            rkFilter.Accepts(EResourceType::AnimSet) || rkFilter.Accepts(EResourceType::Character))
        {
            mpInstance->EvaluateDisplayAsset();
            SetDisplayAsset(mpInstance->DisplayAsset());
        }
        else if (rkFilter.Accepts(EResourceType::DynamicCollision))
        {
            mpInstance->EvaluateCollisionModel();
            mpCollisionNode->SetCollision(mpInstance->Collision());
        }
    }

    // Update other editor properties
    const CScriptTemplate* pTemplate = Template();

    if (pProp == pTemplate->NameProperty())
        SetName("[" + mpInstance->Template()->Name() + "] " + mpInstance->InstanceName());
    else if (pProp == pTemplate->PositionProperty() || pProp->Parent() == pTemplate->PositionProperty())
        mPosition = mpInstance->Position();
    else if (pProp == pTemplate->RotationProperty() || pProp->Parent() == pTemplate->RotationProperty())
        mRotation = CQuaternion::FromEuler(mpInstance->Rotation());
    else if (pProp == pTemplate->ScaleProperty() || pProp->Parent() == pTemplate->ScaleProperty())
        mScale = mpInstance->Scale();

    MarkTransformChanged();
    SetLightLayerIndex(mpLightParameters->LightLayerIndex());

    // Notify attachments
    for (auto* pAttachNode : mAttachments)
    {
        if (pAttachNode->AttachProperty() == pProp)
            pAttachNode->AttachPropertyModified();
    }

    // Notify script extra
    if (mpExtra != nullptr)
        mpExtra->PropertyModified(pProp);

    // Update game mode visibility
    if (pProp != nullptr && pProp == pTemplate->ActiveProperty())
        TestGameModeVisibility();
}

void CScriptNode::UpdatePreviewVolume()
{
    const EVolumeShape Shape = mpInstance->VolumeShape();
    TResPtr<CModel> pVolumeModel;

    switch (Shape)
    {
    case EVolumeShape::AxisAlignedBoxShape:
    case EVolumeShape::BoxShape:
        pVolumeModel = gpEditorStore->LoadResource("VolumeBox.CMDL");
        break;

    case EVolumeShape::EllipsoidShape:
        pVolumeModel = gpEditorStore->LoadResource("VolumeSphere.CMDL");
        break;

    case EVolumeShape::CylinderShape:
        pVolumeModel = gpEditorStore->LoadResource("VolumeCylinder.CMDL");
        break;

    default:
        break;
    }

    mHasVolumePreview = pVolumeModel != nullptr;

    if (mHasVolumePreview)
    {
        mpVolumePreviewNode->SetModel(pVolumeModel);
        mpVolumePreviewNode->SetScale(mpInstance->VolumeScale());
    }
}

void CScriptNode::GeneratePosition()
{
    if (!mHasValidPosition)
    {
        // Default to center of the active area; this is to prevent recursion issues
        const CTransform4f AreaTransform = mpScene->ActiveArea()->Transform();
        mPosition = CVector3f(AreaTransform[0][3], AreaTransform[1][3], AreaTransform[2][3]);
        mHasValidPosition = true;
        MarkTransformChanged();

        // Ideal way to generate the position is to find a spot close to where it's being used.
        // To do this I check the location of the objects that this one is linked to.
        const size_t NumLinks = mpInstance->NumLinks(ELinkType::Incoming) + mpInstance->NumLinks(ELinkType::Outgoing);

        // In the case of one link, apply an offset so the new position isn't the same place as the object it's linked to
        if (NumLinks == 1)
        {
            const uint32 LinkedID = (mpInstance->NumLinks(ELinkType::Incoming) > 0 ? mpInstance->Link(ELinkType::Incoming, 0)->SenderID() : mpInstance->Link(ELinkType::Outgoing, 0)->ReceiverID());
            CScriptNode* pNode = mpScene->NodeForInstanceID(LinkedID);
            pNode->GeneratePosition();
            mPosition = pNode->AbsolutePosition();
            mPosition.Z += (pNode->AABox().Size().Z / 2.f);
            mPosition.Z += (AABox().Size().Z / 2.f);
            mPosition.Z += 2.f;
        }
        // For two or more links, average out the position of the connected objects.
        else if (NumLinks >= 2)
        {
            CVector3f NewPos = CVector3f::Zero();

            for (size_t iIn = 0; iIn < mpInstance->NumLinks(ELinkType::Incoming); iIn++)
            {
                CScriptNode* pNode = mpScene->NodeForInstanceID(mpInstance->Link(ELinkType::Incoming, iIn)->SenderID());

                if (pNode != nullptr)
                {
                    pNode->GeneratePosition();
                    NewPos += pNode->AABox().Center();
                }
            }

            for (size_t iOut = 0; iOut < mpInstance->NumLinks(ELinkType::Outgoing); iOut++)
            {
                CScriptNode* pNode = mpScene->NodeForInstanceID(mpInstance->Link(ELinkType::Outgoing, iOut)->ReceiverID());

                if (pNode != nullptr)
                {
                    pNode->GeneratePosition();
                    NewPos += pNode->AABox().Center();
                }
            }

            mPosition = NewPos / static_cast<float>(NumLinks);
            mPosition.X += 2.f;
        }

        MarkTransformChanged();
    }
}

void CScriptNode::TestGameModeVisibility()
{
    // Don't render if we don't have an ingame model, or if this is the Prime series and the instance is not active.
    if ((Template()->Game() < EGame::DKCReturns && !mpInstance->IsActive()) || !mpInstance->HasInGameModel())
    {
        mGameModeVisibility = EGameModeVisibility::NotVisible;
    }
    else // If this is Returns, only render if the instance is active OR if it has a near visible activation.
    {
        mGameModeVisibility = (mpInstance->IsActive() || mpInstance->HasNearVisibleActivation()) ? EGameModeVisibility::Visible : EGameModeVisibility::NotVisible;
    }
}

CColor CScriptNode::WireframeColor() const
{
    return CColor::Integral(12, 135, 194);
}

CScriptObject* CScriptNode::Instance() const
{
    return mpInstance;
}

CScriptTemplate* CScriptNode::Template() const
{
    return mpInstance->Template();
}

CScriptExtra* CScriptNode::Extra() const
{
    return mpExtra;
}

CModel* CScriptNode::ActiveModel() const
{
    if (mpDisplayAsset != nullptr)
    {
        if (mpDisplayAsset->Type() == EResourceType::Model)
            return static_cast<CModel*>(mpDisplayAsset.RawPointer());
        if (mpDisplayAsset->Type() == EResourceType::AnimSet || mpDisplayAsset->Type() == EResourceType::Character)
            return static_cast<CAnimSet*>(mpDisplayAsset.RawPointer())->Character(mCharIndex)->pModel;
    }

    return nullptr;
}

CAnimSet* CScriptNode::ActiveAnimSet() const
{
    if (mpDisplayAsset != nullptr && (mpDisplayAsset->Type() == EResourceType::AnimSet || mpDisplayAsset->Type() == EResourceType::Character))
        return static_cast<CAnimSet*>(mpDisplayAsset.RawPointer());

    return nullptr;
}

CSkeleton* CScriptNode::ActiveSkeleton() const
{
    if (const CAnimSet* pSet = ActiveAnimSet())
        return pSet->Character(mCharIndex)->pSkeleton;

    return nullptr;
}

CAnimation* CScriptNode::ActiveAnimation() const
{
    if (const CAnimSet* pSet = ActiveAnimSet())
        return pSet->FindAnimationAsset(mAnimIndex);

    return nullptr;
}

CTexture* CScriptNode::ActiveBillboard() const
{
    if (mpDisplayAsset != nullptr && mpDisplayAsset->Type() == EResourceType::Texture)
        return static_cast<CTexture*>(mpDisplayAsset.RawPointer());

    return nullptr;
}

bool CScriptNode::UsesModel() const
{
    return mpDisplayAsset == nullptr || ActiveModel() != nullptr;
}

bool CScriptNode::HasPreviewVolume() const
{
    return mHasVolumePreview;
}

CAABox CScriptNode::PreviewVolumeAABox() const
{
    if (!mHasVolumePreview)
        return CAABox::Zero();

    return mpVolumePreviewNode->AABox();
}

CVector2f CScriptNode::BillboardScale() const
{
    const CVector2f Out = (Template()->ScaleType() == CScriptTemplate::EScaleType::ScaleEnabled ? AbsoluteScale().XZ() : CVector2f(1.f));
    return Out * 0.5f * Template()->PreviewScale();
}

CTransform4f CScriptNode::BoneTransform(uint32 BoneID, EAttachType AttachType, bool Absolute) const
{
    CTransform4f Out;

    if (const CSkeleton* pSkel = ActiveSkeleton())
    {
        const CBone* pBone = pSkel->BoneByID(BoneID);
        ASSERT(pBone);

        if (AttachType == EAttachType::Attach)
            Out.Rotate(pBone->Rotation());

        Out.Translate(pBone->Position());
    }

    if (Absolute)
        Out = Transform() * Out;

    return Out;
}

// ************ PROTECTED ************
void CScriptNode::SetDisplayAsset(CResource *pRes)
{
    mpDisplayAsset = pRes;

    const bool IsAnimSet = pRes != nullptr && (pRes->Type() == EResourceType::AnimSet || pRes->Type() == EResourceType::Character);
    mCharIndex = IsAnimSet ? mpInstance->ActiveCharIndex() : UINT32_MAX;
    mAnimIndex = IsAnimSet ? mpInstance->ActiveAnimIndex() : UINT32_MAX;

    const CModel* pModel = ActiveModel();
    mLocalAABox = pModel != nullptr ? pModel->AABox() : CAABox::One();
    MarkTransformChanged();

    for (auto& attachment : mAttachments)
        attachment->ParentDisplayAssetChanged(pRes);

    if (mpExtra != nullptr)
        mpExtra->DisplayAssetChanged(pRes);
}

void CScriptNode::CalculateTransform(CTransform4f& rOut) const
{
    const CScriptTemplate* pTemp = Template();

    if (pTemp->ScaleType() != CScriptTemplate::EScaleType::ScaleDisabled)
    {
        const CVector3f Scale = (HasPreviewVolume() ? CVector3f::One() : AbsoluteScale());
        rOut.Scale(Scale * pTemp->PreviewScale());
    }

    if (pTemp->RotationType() == CScriptTemplate::ERotationType::RotationEnabled)
        rOut.Rotate(AbsoluteRotation());

    rOut.Translate(AbsolutePosition());
}
