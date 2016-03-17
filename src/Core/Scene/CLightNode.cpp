#include "CLightNode.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CGraphics.h"
#include "Core/Render/CRenderer.h"
#include <Math/MathUtil.h>

CLightNode::CLightNode(CScene *pScene, u32 NodeID, CSceneNode *pParent, CLight *Light)
    : CSceneNode(pScene, NodeID, pParent)
{
    mpLight = Light;
    mLocalAABox = CAABox::skOne;
    mPosition = Light->GetPosition();

    switch (Light->GetType())
    {
    case eLocalAmbient: SetName("Ambient Light");     break;
    case eDirectional:  SetName("Directional Light"); break;
    case eSpot:         SetName("Spot Light");        break;
    case eCustom:       SetName("Custom Light");      break;
    }
}

ENodeType CLightNode::NodeType()
{
    return eLightNode;
}

void CLightNode::AddToRenderer(CRenderer *pRenderer, const SViewInfo& ViewInfo)
{
    if (ViewInfo.GameMode) return;

    if (ViewInfo.ViewFrustum.BoxInFrustum(AABox()))
        pRenderer->AddOpaqueMesh(this, -1, AABox(), eDrawMesh);

    if (IsSelected() && mpLight->GetType() == eCustom)
    {
        CAABox RadiusBox = (CAABox::skOne * 2.f * mpLight->GetRadius()) + mPosition;

        if (ViewInfo.ViewFrustum.BoxInFrustum(RadiusBox))
            pRenderer->AddOpaqueMesh(this, -1, AABox(), eDrawSelection);
    }
}

void CLightNode::Draw(FRenderOptions /*Options*/, int /*ComponentIndex*/, const SViewInfo& ViewInfo)
{
    CDrawUtil::DrawLightBillboard(mpLight->GetType(), mpLight->GetColor(), mPosition, BillboardScale(), TintColor(ViewInfo));
}

void CLightNode::DrawSelection()
{
    CDrawUtil::DrawWireSphere(mPosition, mpLight->GetRadius(), mpLight->GetColor());
}

void CLightNode::RayAABoxIntersectTest(CRayCollisionTester& Tester, const SViewInfo& /*ViewInfo*/)
{
    CVector2f BillScale = BillboardScale();
    float ScaleXY = (BillScale.x > BillScale.y ? BillScale.x : BillScale.y);

    CAABox BillBox = CAABox(mPosition + CVector3f(-ScaleXY, -ScaleXY, -BillScale.y),
                            mPosition + CVector3f( ScaleXY,  ScaleXY,  BillScale.y));

    std::pair<bool,float> BoxResult = BillBox.IntersectsRay(Tester.Ray());
    if (BoxResult.first) Tester.AddNode(this, 0, BoxResult.second);
}

SRayIntersection CLightNode::RayNodeIntersectTest(const CRay& Ray, u32 AssetID, const SViewInfo& ViewInfo)
{
    // todo: come up with a better way to share this code between CScriptNode and CLightNode
    SRayIntersection out;
    out.pNode = this;
    out.ComponentIndex = AssetID;

    CTexture *pBillboard = CDrawUtil::GetLightTexture(mpLight->GetType());

    if (!pBillboard)
    {
        out.Hit = false;
        return out;
    }

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
            float TexelAlpha = pBillboard->ReadTexelAlpha(TexCoord);

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

    return out;
}

CLight* CLightNode::Light()
{
    return mpLight;
}

CVector2f CLightNode::BillboardScale()
{
    return AbsoluteScale().xz() * 0.75f;
}

void CLightNode::CalculateTransform(CTransform4f& rOut) const
{
    // Billboards don't rotate and their scale is applied separately
    rOut.Translate(AbsolutePosition());
}
