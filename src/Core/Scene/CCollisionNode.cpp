#include "CCollisionNode.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CGraphics.h"
#include "Core/Render/CRenderer.h"

CCollisionNode::CCollisionNode(CScene *pScene, u32 NodeID, CSceneNode *pParent, CCollisionMeshGroup *pCollision)
    : CSceneNode(pScene, NodeID, pParent)
{
    SetCollision(pCollision);
    SetName("Collision");
}

ENodeType CCollisionNode::NodeType()
{
    return eCollisionNode;
}

void CCollisionNode::AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo)
{
    if (!mpCollision) return;
    if (!rkViewInfo.ViewFrustum.BoxInFrustum(AABox())) return;
    if (rkViewInfo.GameMode) return;

    pRenderer->AddMesh(this, -1, AABox(), false, eDrawMesh);

    if (mSelected)
        pRenderer->AddMesh(this, -1, AABox(), false, eDrawSelection);
}

void CCollisionNode::Draw(FRenderOptions /*Options*/, int /*ComponentIndex*/, ERenderCommand /*Command*/, const SViewInfo& rkViewInfo)
{
    if (!mpCollision) return;

    LoadModelMatrix();

    glBlendFunc(GL_ONE, GL_ZERO);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);

    CColor BaseTint = TintColor(rkViewInfo);

    for (u32 iMesh = 0; iMesh < mpCollision->NumMeshes(); iMesh++)
    {
        CCollisionMesh *pMesh = mpCollision->MeshByIndex(iMesh);

        for (u32 iMat = 0; iMat < pMesh->NumMaterials(); iMat++)
        {
            SCollisionMaterial& rMat = pMesh->GetMaterial(iMat);

            if (rkViewInfo.CollisionSettings.HideMask != 0 && (rMat.RawFlags & rkViewInfo.CollisionSettings.HideMask) == rkViewInfo.CollisionSettings.HideMask)
                continue;

            CColor Tint = BaseTint;

            if (rkViewInfo.CollisionSettings.HighlightMask != 0 && (rMat.RawFlags & rkViewInfo.CollisionSettings.HighlightMask) == rkViewInfo.CollisionSettings.HighlightMask)
                Tint *= CColor::skRed;

            else if (mpCollision->Game() <= ePrime && rkViewInfo.CollisionSettings.DrawMode == eCDM_TintSurfaceType)
            {
                // Organic
                if (rMat.RawFlags & 0x00800000)
                    Tint *= CColor::Integral(130, 130, 250); // Purple
                // Wood
                else if (rMat.RawFlags & 0x00400000)
                    Tint *= CColor::Integral(190, 140, 105); // Brown
                // Sand
                else if (rMat.RawFlags & 0x00020000)
                    Tint *= CColor::Integral(230, 200, 170); // Light Brown
                // Shield
                else if (rMat.RawFlags & 0x00010000)
                    Tint *= CColor::Integral(250, 230, 60); // Yellow
                // Glass
                else if (rMat.RawFlags & 0x00008000)
                    Tint *= CColor::Integral(20, 255, 190); // Greenish/Bluish
                // Snow
                else if (rMat.RawFlags & 0x00000800)
                    Tint *= CColor::Integral(230, 255, 255); // *Very* light blue
                // Lava
                else if (rMat.RawFlags & 0x00000200)
                    Tint *= CColor::Integral(200, 30, 30); // Red
                // Rock
                else if (rMat.RawFlags & 0x00000100)
                    Tint *= CColor::Integral(150, 130, 120); // Brownish-gray
                // Phazon
                else if (rMat.RawFlags & 0x00000080)
                    Tint *= CColor::Integral(0, 128, 255); // Blue
                // Metal Grating
                else if (rMat.RawFlags & 0x00000040)
                    Tint *= CColor::Integral(170, 170, 170); // Gray
                // Ice
                else if (rMat.RawFlags & 0x00000010)
                    Tint *= CColor::Integral(200, 255, 255); // Light blue
                // Grass
                else if (rMat.RawFlags & 0x00000008)
                    Tint *= CColor::Integral(90, 150, 70); // Green
                // Metal
                else if (rMat.RawFlags & 0x00000004)
                    Tint *= CColor::Integral(110, 110, 110); // Dark gray
                // Stone
                else if (rMat.RawFlags & 0x00000002)
                    Tint *= CColor::Integral(220, 215, 160); // Brown/green ish
            }

            CDrawUtil::UseCollisionShader(Tint);
            pMesh->DrawMaterial(iMat);
        }
    }

    if (rkViewInfo.CollisionSettings.DrawWireframe)
    {
        CDrawUtil::UseColorShader(CColor::skTransparentBlack);
        mpCollision->DrawWireframe();
    }
}

SRayIntersection CCollisionNode::RayNodeIntersectTest(const CRay& /*rkRay*/, u32 /*AssetID*/, const SViewInfo& /*rkViewInfo*/)
{
    // todo
    SRayIntersection Result;
    Result.Hit = false;
    return Result;
}

void CCollisionNode::SetCollision(CCollisionMeshGroup *pCollision)
{
    mpCollision = pCollision;
}
