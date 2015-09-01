#include "CScriptNode.h"
#include <gtx/quaternion.hpp>
#include <Common/AnimUtil.h>
#include <Common/Math.h>
#include <Core/CDrawUtil.h>
#include <Core/CGraphics.h>
#include <Core/CRenderer.h>
#include <Core/CResCache.h>
#include <Core/CSceneManager.h>

CScriptNode::CScriptNode(CSceneManager *pScene, CSceneNode *pParent, CScriptObject *pObject)
    : CSceneNode(pScene, pParent)
{
    mpVolumePreviewNode = nullptr;

    // Evaluate instance
    mpInstance = pObject;
    mpActiveModel = nullptr;

    if (mpInstance)
    {
        mpActiveModel = mpInstance->GetDisplayModel();
        mPosition = mpInstance->GetPosition();
        mRotation = CQuaternion::FromEuler(mpInstance->GetRotation());
        mScale = mpInstance->GetScale();
        SetName("[" + mpInstance->Template()->TemplateName() + "] " + mpInstance->GetInstanceName());
        MarkTransformChanged();

        mHasValidPosition = ((mpInstance->GetAttribFlags() & ePositionAttrib) != 0);
        mHasVolumePreview = ((mpInstance->GetAttribFlags() & eVolumeAttrib) != 0);

        // Create volume preview node
        if (mHasVolumePreview)
        {
            u32 VolumeShape = mpInstance->GetVolumeShape();
            CModel *pVolumeModel = nullptr;

            if ((VolumeShape == 0) || (VolumeShape == 1)) // Box/OrientedBox
                pVolumeModel = (CModel*) gResCache.GetResource("../resources/VolumeBox.cmdl");

            else if (VolumeShape == 2) // Sphere
                pVolumeModel = (CModel*) gResCache.GetResource("../resources/VolumeSphere.cmdl");

            if (pVolumeModel)
            {
                mpVolumePreviewNode = new CModelNode(pScene, this, pVolumeModel);
                mpVolumePreviewNode->SetInheritance(true, (VolumeShape == 1), false);
                mpVolumePreviewNode->Scale(mpInstance->GetVolume());
                mpVolumePreviewNode->ForceAlphaEnabled(true);
            }
        }
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
}

ENodeType CScriptNode::NodeType()
{
    return eScriptNode;
}

std::string CScriptNode::PrefixedName() const
{
    return "[" + mpInstance->Template()->TemplateName() + "] " + mpInstance->GetInstanceName();
}

void CScriptNode::AddToRenderer(CRenderer *pRenderer)
{
    if (!mpInstance) return;

    if (!mpActiveModel)
        pRenderer->AddOpaqueMesh(this, 0, AABox(), eDrawMesh);

    else
    {
        if (!mpActiveModel->IsBuffered())
            mpActiveModel->BufferGL();

        if (!mpActiveModel->HasTransparency(0))
            pRenderer->AddOpaqueMesh(this, 0, AABox(), eDrawMesh);

        else
        {
            u32 SubmeshCount = mpActiveModel->GetSurfaceCount();

            for (u32 s = 0; s < SubmeshCount; s++)
            {
                if (!mpActiveModel->IsSurfaceTransparent(s, 0))
                    pRenderer->AddOpaqueMesh(this, s, mpActiveModel->GetSurfaceAABox(s).Transformed(Transform()), eDrawAsset);
                else
                    pRenderer->AddTransparentMesh(this, s, mpActiveModel->GetSurfaceAABox(s).Transformed(Transform()), eDrawAsset);
            }
        }
    }

    if (IsSelected())
    {
        pRenderer->AddOpaqueMesh(this, 0, AABox(), eDrawSelection);

        if (mHasVolumePreview)
            mpVolumePreviewNode->AddToRenderer(pRenderer);
    }
}

void CScriptNode::Draw(ERenderOptions Options)
{
    if (!mpInstance) return;

    // Set lighting
    LoadModelMatrix();
    LoadLights();

    if (CGraphics::sLightMode == CGraphics::WorldLighting)
        CGraphics::sVertexBlock.COLOR0_Amb = CGraphics::sAreaAmbientColor.ToVector4f() * CGraphics::sWorldLightMultiplier;
    else
        CGraphics::sVertexBlock.COLOR0_Amb = CGraphics::skDefaultAmbientColor.ToVector4f();

    // Default to drawing purple box if no model
    if (!mpActiveModel)
    {
        glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ZERO, GL_ZERO);
        glDepthMask(GL_TRUE);

        LoadModelMatrix();
        CGraphics::UpdateVertexBlock();
        CGraphics::UpdateLightBlock();
        CDrawUtil::DrawShadedCube(CColor::skTransparentPurple);
        return;
    }

    // Set tev color (used rarely)
    CGraphics::sPixelBlock.TevColor = mpInstance->GetTevColor().ToVector4f();

    mpActiveModel->Draw(Options, 0);
}

void CScriptNode::DrawAsset(ERenderOptions Options, u32 Asset)
{
    if (!mpInstance) return;
    if (!mpActiveModel) return;

    if (CGraphics::sLightMode == CGraphics::WorldLighting)
        CGraphics::sVertexBlock.COLOR0_Amb = CGraphics::sAreaAmbientColor.ToVector4f() * CGraphics::sWorldLightMultiplier;
    else
        CGraphics::sVertexBlock.COLOR0_Amb = CGraphics::skDefaultAmbientColor.ToVector4f();

    LoadModelMatrix();
    LoadLights();

    CGraphics::sPixelBlock.TevColor = mpInstance->GetTevColor().ToVector4f();

    mpActiveModel->DrawSurface(Options, Asset, 0);
}

void CScriptNode::DrawSelection()
{
    glBlendFunc(GL_ONE, GL_ZERO);
    CDrawUtil::DrawWireCube(AABox(), CColor::skTransparentWhite);

    if (mpInstance)
    {
        CGraphics::sMVPBlock.ModelMatrix = CMatrix4f::skIdentity;
        CGraphics::UpdateMVPBlock();

        for (u32 iIn = 0; iIn < mpInstance->NumInLinks(); iIn++)
        {
            const SLink& con = mpInstance->InLink(iIn);
            CScriptNode *pLinkNode = mpScene->ScriptNodeByID(con.ObjectID);
            if (pLinkNode) CDrawUtil::DrawLine(CenterPoint(), pLinkNode->CenterPoint(), CColor::skTransparentRed);
        }

        for (u32 iOut = 0; iOut < mpInstance->NumOutLinks(); iOut++)
        {
            const SLink& con = mpInstance->OutLink(iOut);
            CScriptNode *pLinkNode = mpScene->ScriptNodeByID(con.ObjectID);
            if (pLinkNode) CDrawUtil::DrawLine(CenterPoint(), pLinkNode->CenterPoint(), CColor::skTransparentGreen);
        }
    }
}

void CScriptNode::RayAABoxIntersectTest(CRayCollisionTester &Tester)
{
    if (!mpInstance)
        return;

    const CRay& Ray = Tester.Ray();
    std::pair<bool,float> BoxResult = AABox().IntersectsRay(Ray);

    if (BoxResult.first)
    {
        if (mpActiveModel)
        {
            for (u32 iSurf = 0; iSurf < mpActiveModel->GetSurfaceCount(); iSurf++)
            {
                std::pair<bool,float> SurfResult = mpActiveModel->GetSurfaceAABox(iSurf).Transformed(Transform()).IntersectsRay(Ray);

                if (SurfResult.first)
                    Tester.AddNode(this, iSurf, SurfResult.second);
            }
        }
        else Tester.AddNode(this, 0, BoxResult.second);
    }
}

SRayIntersection CScriptNode::RayNodeIntersectTest(const CRay &Ray, u32 AssetID)
{
    SRayIntersection out;
    out.pNode = this;
    out.AssetIndex = AssetID;

    CRay TransformedRay = Ray.Transformed(Transform().Inverse());
    CModel *pModel = (mpActiveModel ? mpActiveModel : CDrawUtil::GetCubeModel());
    std::pair<bool,float> Result = pModel->GetSurface(AssetID)->IntersectsRay(TransformedRay);

    if (Result.first)
    {
        out.Hit = true;

        CVector3f HitPoint = TransformedRay.PointOnRay(Result.second);
        CVector3f WorldHitPoint = Transform() * HitPoint;
        out.Distance = Math::Distance(Ray.Origin(), WorldHitPoint);
    }

    else
        out.Hit = false;

    return out;
}

bool CScriptNode::IsVisible() const
{
    // Reimplementation of CSceneNode::IsHidden() to allow for layer and template visiblity to be taken into account
    return (mVisible && mpInstance->Layer()->IsVisible() && mpInstance->Template()->IsVisible());
}

CScriptObject* CScriptNode::Object()
{
    return mpInstance;
}

CModel* CScriptNode::ActiveModel()
{
    return mpActiveModel;
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

            mPosition = NewPos / NumLinks;
            mPosition.x += 2.f;
        }

        MarkTransformChanged();
    }
}

bool CScriptNode::HasPreviewVolume()
{
    return mHasVolumePreview;
}

CAABox CScriptNode::PreviewVolumeAABox()
{
    if (!mHasVolumePreview)
        return CAABox::skZero;
    else
        return mpVolumePreviewNode->AABox();
}
