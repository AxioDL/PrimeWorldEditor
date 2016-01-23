#include "CScriptNode.h"
#include "CScene.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CGraphics.h"
#include "Core/Render/CRenderer.h"
#include "Core/Resource/CResCache.h"
#include "Core/Resource/Script/CMasterTemplate.h"
#include "Core/Resource/Script/CScriptLayer.h"
#include "Core/ScriptExtra/CScriptExtra.h"
#include <Common/AnimUtil.h>
#include <Math/MathUtil.h>

CScriptNode::CScriptNode(CScene *pScene, CSceneNode *pParent, CScriptObject *pObject)
    : CSceneNode(pScene, pParent)
{
    mpVolumePreviewNode = nullptr;
    mHasVolumePreview = false;

    // Evaluate instance
    mpInstance = pObject;
    mpActiveModel = nullptr;
    mpBillboard = nullptr;
    mpCollisionNode = new CCollisionNode(pScene, this);
    mpCollisionNode->SetInheritance(true, true, false);

    if (mpInstance)
    {
        CScriptTemplate *pTemp = Template();

        // Determine transform
        mPosition = mpInstance->Position();
        mRotation = CQuaternion::FromEuler(mpInstance->Rotation());
        mScale = mpInstance->Scale();
        MarkTransformChanged();

        SetName("[" + pTemp->Name() + "] " + mpInstance->InstanceName());

        // Determine display assets
        mpActiveModel = mpInstance->GetDisplayModel();
        mpBillboard = mpInstance->GetBillboard();
        mpCollisionNode->SetCollision(mpInstance->GetCollision());

        // Create preview volume node
        mHasValidPosition = pTemp->HasPosition();

        if (pTemp->ScaleType() == CScriptTemplate::eScaleVolume)
        {
            EVolumeShape shape = mpInstance->VolumeShape();
            TResPtr<CModel> pVolumeModel = nullptr;

            switch (shape)
            {
            case eAxisAlignedBoxShape:
            case eBoxShape:
                pVolumeModel = gResCache.GetResource("../resources/VolumeBox.cmdl");
                break;

            case eEllipsoidShape:
                pVolumeModel = gResCache.GetResource("../resources/VolumeSphere.cmdl");
                break;

            case eCylinderShape:
                pVolumeModel = gResCache.GetResource("../resources/VolumeCylinder.cmdl");
                break;
            }

            mHasVolumePreview = (pVolumeModel != nullptr);

            if (mHasVolumePreview)
            {
                mpVolumePreviewNode = new CModelNode(pScene, this, pVolumeModel);
                mpVolumePreviewNode->SetInheritance(true, (shape != eAxisAlignedBoxShape), true);
                mpVolumePreviewNode->SetScale(mpInstance->VolumeScale());
                mpVolumePreviewNode->ForceAlphaEnabled(true);
            }
        }

        // Fetch LightParameters
        mpLightParameters = new CLightParameters(mpInstance->LightParameters(), mpInstance->MasterTemplate()->GetGame());
        SetLightLayerIndex(mpLightParameters->LightLayerIndex());
    }

    else
    {
        // Shouldn't ever happen
        SetName("ScriptNode - NO INSTANCE");
    }

    if (mpActiveModel)
        mLocalAABox = mpActiveModel->AABox();
    else
        mLocalAABox = CAABox::skOne;

    mpExtra = CScriptExtra::CreateExtra(this);
}

ENodeType CScriptNode::NodeType()
{
    return eScriptNode;
}

void CScriptNode::AddToRenderer(CRenderer *pRenderer, const SViewInfo& ViewInfo)
{
    if (!mpInstance) return;

    // Add script extra to renderer first
    if (mpExtra) mpExtra->AddToRenderer(pRenderer, ViewInfo);

    // If we're in game mode, then override other visibility settings.
    if (ViewInfo.GameMode)
    {
        if ( (!mpInstance->IsActive() && Template()->Game() != eReturns) || !mpInstance->HasInGameModel())
            return;
    }

    // Check whether the script extra wants us to render before we render.
    bool ShouldDraw = (!mpExtra || mpExtra->ShouldDrawNormalAssets());

    if (ShouldDraw)
    {
        // Otherwise, we proceed as normal
        if ((ViewInfo.ShowFlags & eShowObjectCollision) && (!ViewInfo.GameMode))
            mpCollisionNode->AddToRenderer(pRenderer, ViewInfo);

        if (ViewInfo.ShowFlags & eShowObjectGeometry || ViewInfo.GameMode)
        {
            if (ViewInfo.ViewFrustum.BoxInFrustum(AABox()))
            {
                if (!mpActiveModel)
                    pRenderer->AddOpaqueMesh(this, -1, AABox(), eDrawMesh);

                else
                {
                    if (!mpActiveModel->HasTransparency(0))
                        pRenderer->AddOpaqueMesh(this, -1, AABox(), eDrawMesh);
                    else
                        AddSurfacesToRenderer(pRenderer, mpActiveModel, 0, ViewInfo);
                }
            }
        }
    }

    if (IsSelected() && !ViewInfo.GameMode)
    {
        // Script nodes always draw their selections regardless of frustum planes
        // in order to ensure that script connection lines don't get improperly culled.
       if (ShouldDraw)
           pRenderer->AddOpaqueMesh(this, -1, AABox(), eDrawSelection);

        if (mHasVolumePreview && (!mpExtra || mpExtra->ShouldDrawVolume()))
            mpVolumePreviewNode->AddToRenderer(pRenderer, ViewInfo);
    }
}

void CScriptNode::Draw(FRenderOptions Options, int ComponentIndex, const SViewInfo& ViewInfo)
{
    if (!mpInstance) return;

    // Draw model
    if (UsesModel())
    {
        CGraphics::SetupAmbientColor();
        LoadModelMatrix();
        LoadLights(ViewInfo);
        CGraphics::UpdateVertexBlock();

        // Draw model if possible!
        if (mpActiveModel)
        {
            if (mpExtra) CGraphics::sPixelBlock.TevColor = mpExtra->TevColor();
            else CGraphics::sPixelBlock.TevColor = CColor::skWhite;
            CGraphics::sPixelBlock.TintColor = TintColor(ViewInfo);
            CGraphics::UpdatePixelBlock();

            if (ComponentIndex < 0)
                mpActiveModel->Draw(Options, 0);
            else
                mpActiveModel->DrawSurface(Options, ComponentIndex, 0);
        }

        // If no model or billboard, default to drawing a purple box
        else
        {
            glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ZERO, GL_ZERO);
            glDepthMask(GL_TRUE);
            CDrawUtil::DrawShadedCube(CColor::skTransparentPurple * TintColor(ViewInfo));
        }
    }

    // Draw billboard
    else if (mpBillboard)
    {
        CDrawUtil::DrawBillboard(mpBillboard, mPosition, BillboardScale(), TintColor(ViewInfo));
    }
}

void CScriptNode::DrawSelection()
{
    glBlendFunc(GL_ONE, GL_ZERO);

    // Draw wireframe for models; billboards only get tinted
    if (UsesModel())
    {
        LoadModelMatrix();
        CModel *pModel = (mpActiveModel ? mpActiveModel : CDrawUtil::GetCubeModel());
        pModel->DrawWireframe(eNoRenderOptions, WireframeColor());
    }

    if (mpInstance)
    {
        CGraphics::sMVPBlock.ModelMatrix = CMatrix4f::skIdentity;
        CGraphics::UpdateMVPBlock();

        for (u32 iIn = 0; iIn < mpInstance->NumInLinks(); iIn++)
        {
            // Don't draw in links if the other object is selected.
            const SLink& con = mpInstance->InLink(iIn);
            CScriptNode *pLinkNode = mpScene->ScriptNodeByID(con.ObjectID);
            if (pLinkNode && !pLinkNode->IsSelected()) CDrawUtil::DrawLine(CenterPoint(), pLinkNode->CenterPoint(), CColor::skTransparentRed);
        }

        for (u32 iOut = 0; iOut < mpInstance->NumOutLinks(); iOut++)
        {
            const SLink& con = mpInstance->OutLink(iOut);
            CScriptNode *pLinkNode = mpScene->ScriptNodeByID(con.ObjectID);
            if (pLinkNode) CDrawUtil::DrawLine(CenterPoint(), pLinkNode->CenterPoint(), CColor::skTransparentGreen);
        }
    }
}

void CScriptNode::RayAABoxIntersectTest(CRayCollisionTester& Tester, const SViewInfo& ViewInfo)
{
    if (!mpInstance)
        return;

    // Let script extra do ray check first
    if (mpExtra)
    {
        mpExtra->RayAABoxIntersectTest(Tester, ViewInfo);

        // If the extra doesn't want us rendering, then don't do the ray test either
        if (!mpExtra->ShouldDrawNormalAssets())
            return;
    }

    // If we're in game mode, then check whether we're visible before proceeding with the ray test.
    if (ViewInfo.GameMode)
    {
        if ( (!mpInstance->IsActive() && Template()->Game() != eReturns) || !mpInstance->HasInGameModel())
            return;
    }

    // Otherwise, proceed with the ray test as normal...
    const CRay& Ray = Tester.Ray();

    if (UsesModel())
    {
        std::pair<bool,float> BoxResult = AABox().IntersectsRay(Ray);

        if (BoxResult.first)
        {
            if (mpActiveModel) Tester.AddNodeModel(this, mpActiveModel);
            else Tester.AddNode(this, 0, BoxResult.second);
        }
    }

    else
    {
        // Because the billboard rotates a lot, expand the AABox on the X/Y axes to cover any possible orientation
        CVector2f BillScale = BillboardScale();
        float ScaleXY = (BillScale.x > BillScale.y ? BillScale.x : BillScale.y);

        CAABox BillBox = CAABox(mPosition + CVector3f(-ScaleXY, -ScaleXY, -BillScale.y),
                                mPosition + CVector3f( ScaleXY,  ScaleXY,  BillScale.y));

        std::pair<bool,float> BoxResult = BillBox.IntersectsRay(Ray);
        if (BoxResult.first) Tester.AddNode(this, 0, BoxResult.second);
    }
}

SRayIntersection CScriptNode::RayNodeIntersectTest(const CRay& Ray, u32 AssetID, const SViewInfo& ViewInfo)
{
    FRenderOptions Options = ViewInfo.pRenderer->RenderOptions();

    SRayIntersection out;
    out.pNode = this;
    out.ComponentIndex = AssetID;

    // Model test
    if (UsesModel())
    {
        CModel *pModel = (mpActiveModel ? mpActiveModel : CDrawUtil::GetCubeModel());

        CRay TransformedRay = Ray.Transformed(Transform().Inverse());
        std::pair<bool,float> Result = pModel->GetSurface(AssetID)->IntersectsRay(TransformedRay, ((Options & eEnableBackfaceCull) == 0));

        if (Result.first)
        {
            out.Hit = true;

            CVector3f HitPoint = TransformedRay.PointOnRay(Result.second);
            CVector3f WorldHitPoint = Transform() * HitPoint;
            out.Distance = Math::Distance(Ray.Origin(), WorldHitPoint);
        }

        else
            out.Hit = false;
    }

    // Billboard test
    // todo: come up with a better way to share this code between CScriptNode and CLightNode
    else
    {
        // Step 1: check whether the ray intersects with the plane the billboard is on
        CPlane BillboardPlane(-ViewInfo.pCamera->Direction(), mPosition);
        std::pair<bool,float> PlaneTest = Math::RayPlaneIntersecton(Ray, BillboardPlane);

        if (PlaneTest.first)
        {
            // Step 2: transform the hit point into the plane's local space
            CVector3f PlaneHitPoint = Ray.PointOnRay(PlaneTest.second);
            CVector3f RelHitPoint = PlaneHitPoint - mPosition;

            CVector3f PlaneForward = -ViewInfo.pCamera->Direction();
            CVector3f PlaneRight = -ViewInfo.pCamera->RightVector();
            CVector3f PlaneUp = ViewInfo.pCamera->UpVector();
            CQuaternion PlaneRot = CQuaternion::FromAxes(PlaneRight, PlaneForward, PlaneUp);

            CVector3f RotatedHitPoint = PlaneRot.Inverse() * RelHitPoint;
            CVector2f LocalHitPoint = RotatedHitPoint.xz() / BillboardScale();

            // Step 3: check whether the transformed hit point is in the -1 to 1 range
            if ((LocalHitPoint.x >= -1.f) && (LocalHitPoint.x <= 1.f) && (LocalHitPoint.y >= -1.f) && (LocalHitPoint.y <= 1.f))
            {
                // Step 4: look up the hit texel and check whether it's transparent or opaque
                CVector2f TexCoord = (LocalHitPoint + CVector2f(1.f)) * 0.5f;
                TexCoord.x = -TexCoord.x + 1.f;
                float TexelAlpha = mpBillboard->ReadTexelAlpha(TexCoord);

                if (TexelAlpha < 0.25f)
                    out.Hit = false;

                else
                {
                    // It's opaque... we have a hit!
                    out.Hit = true;
                    out.Distance = PlaneTest.second;
                }
            }

            else
                out.Hit = false;
        }

        else
            out.Hit = false;
    }

    return out;
}

bool CScriptNode::AllowsRotate() const
{
    return (Template()->RotationType() == CScriptTemplate::eRotationEnabled);
}

bool CScriptNode::AllowsScale() const
{
    return (Template()->ScaleType() != CScriptTemplate::eScaleDisabled);
}

bool CScriptNode::IsVisible() const
{
    // Reimplementation of CSceneNode::IsVisible() to allow for layer and template visiblity to be taken into account
    return (mVisible && mpInstance->Layer()->IsVisible() && Template()->IsVisible());
}

CColor CScriptNode::TintColor(const SViewInfo &ViewInfo) const
{
    CColor BaseColor = CSceneNode::TintColor(ViewInfo);
    if (mpExtra) mpExtra->ModifyTintColor(BaseColor);
    return BaseColor;
}

void CScriptNode::GeneratePosition()
{
    if  (!mHasValidPosition)
    {
        // Default to center of the active area; this is to preven recursion issues
        CTransform4f& AreaTransform = mpScene->GetActiveArea()->GetTransform();
        mPosition = CVector3f(AreaTransform[0][3], AreaTransform[1][3], AreaTransform[2][3]);
        mHasValidPosition = true;
        MarkTransformChanged();

        // Ideal way to generate the position is to find a spot close to where it's being used.
        // To do this I check the location of the objects that this one is linked to.
        u32 NumLinks = mpInstance->NumInLinks() + mpInstance->NumOutLinks();

        // In the case of one link, apply an offset so the new position isn't the same place as the object it's linked to
        if (NumLinks == 1)
        {
            const SLink& link = (mpInstance->NumInLinks() > 0 ? mpInstance->InLink(0) : mpInstance->OutLink(0));
            CScriptNode *pNode = mpScene->ScriptNodeByID(link.ObjectID);
            pNode->GeneratePosition();
            mPosition = pNode->AbsolutePosition();
            mPosition.z += (pNode->AABox().Size().z / 2.f);
            mPosition.z += (AABox().Size().z / 2.f);
            mPosition.z += 2.f;
        }

        // For two or more links, average out the position of the connected objects.
        else if (NumLinks >= 2)
        {
            CVector3f NewPos = CVector3f::skZero;

            for (u32 iIn = 0; iIn < mpInstance->NumInLinks(); iIn++)
            {
                CScriptNode *pNode = mpScene->ScriptNodeByID(mpInstance->InLink(iIn).ObjectID);

                if (pNode)
                {
                    pNode->GeneratePosition();
                    NewPos += pNode->AABox().Center();
                }
            }

            for (u32 iOut = 0; iOut < mpInstance->NumOutLinks(); iOut++)
            {
                CScriptNode *pNode = mpScene->ScriptNodeByID(mpInstance->OutLink(iOut).ObjectID);

                if (pNode)
                {
                    pNode->GeneratePosition();
                    NewPos += pNode->AABox().Center();
                }
            }

            mPosition = NewPos / (float) NumLinks;
            mPosition.x += 2.f;
        }

        MarkTransformChanged();
    }
}

CColor CScriptNode::WireframeColor() const
{
    return CColor::Integral(12, 135, 194);
}

CScriptObject* CScriptNode::Object() const
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
    return mpActiveModel;
}

bool CScriptNode::UsesModel() const
{
    return ((mpActiveModel != nullptr) || (mpBillboard == nullptr));
}

bool CScriptNode::HasPreviewVolume() const
{
    return mHasVolumePreview;
}

CAABox CScriptNode::PreviewVolumeAABox() const
{
    if (!mHasVolumePreview)
        return CAABox::skZero;
    else
        return mpVolumePreviewNode->AABox();
}

CVector2f CScriptNode::BillboardScale() const
{
    CVector2f out = (Template()->ScaleType() == CScriptTemplate::eScaleEnabled ? AbsoluteScale().xz() : CVector2f(1.f));
    return out * 0.5f * Template()->PreviewScale();
}

// ************ PROTECTED ************
void CScriptNode::CalculateTransform(CTransform4f& rOut) const
{
    CScriptTemplate *pTemp = Template();

    if (pTemp->ScaleType() != CScriptTemplate::eScaleDisabled)
    {
        CVector3f Scale = (HasPreviewVolume() ? CVector3f::skOne : AbsoluteScale());
        rOut.Scale(Scale * pTemp->PreviewScale());
    }

    if (UsesModel() && pTemp->RotationType() == CScriptTemplate::eRotationEnabled)
        rOut.Rotate(AbsoluteRotation());

    rOut.Translate(AbsolutePosition());
}
