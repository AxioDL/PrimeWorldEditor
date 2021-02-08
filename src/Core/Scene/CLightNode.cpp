#include "CLightNode.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CGraphics.h"
#include "Core/Render/CRenderer.h"
#include <Common/Math/MathUtil.h>

CLightNode::CLightNode(CScene *pScene, uint32 NodeID, CSceneNode *pParent, CLight *pLight)
    : CSceneNode(pScene, NodeID, pParent)
    , mpLight(pLight)
{
    mLocalAABox = CAABox::One();
    mPosition = pLight->Position();

    switch (pLight->Type())
    {
    case ELightType::LocalAmbient: SetName("Ambient Light");     break;
    case ELightType::Directional:  SetName("Directional Light"); break;
    case ELightType::Spot:         SetName("Spot Light");        break;
    case ELightType::Custom:       SetName("Custom Light");      break;
    }
}

ENodeType CLightNode::NodeType()
{
    return ENodeType::Light;
}

void CLightNode::AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo)
{
    if (rkViewInfo.GameMode) return;

    if (rkViewInfo.ViewFrustum.BoxInFrustum(AABox()))
        pRenderer->AddMesh(this, -1, AABox(), false, ERenderCommand::DrawMesh);

    if (IsSelected() && mpLight->Type() == ELightType::Custom)
    {
        const CAABox RadiusBox = (CAABox::One() * 2.f * mpLight->GetRadius()) + mPosition;

        if (rkViewInfo.ViewFrustum.BoxInFrustum(RadiusBox))
            pRenderer->AddMesh(this, -1, AABox(), false, ERenderCommand::DrawSelection);
    }
}

void CLightNode::Draw(FRenderOptions /*Options*/, int /*ComponentIndex*/, ERenderCommand /*Command*/, const SViewInfo& rkViewInfo)
{
    CDrawUtil::DrawLightBillboard(mpLight->Type(), mpLight->Color(), mPosition, BillboardScale(), TintColor(rkViewInfo));
}

void CLightNode::DrawSelection()
{
    CDrawUtil::DrawWireSphere(mPosition, mpLight->GetRadius(), mpLight->Color());
}

void CLightNode::RayAABoxIntersectTest(CRayCollisionTester& rTester, const SViewInfo& /*ViewInfo*/)
{
    const CVector2f BillScale = BillboardScale();
    const float ScaleXY = (BillScale.X > BillScale.Y ? BillScale.X : BillScale.Y);

    const CAABox BillBox = CAABox(mPosition + CVector3f(-ScaleXY, -ScaleXY, -BillScale.Y),
                                  mPosition + CVector3f(ScaleXY, ScaleXY, BillScale.Y));

    const auto [intersects, distance] = BillBox.IntersectsRay(rTester.Ray());
    if (intersects)
        rTester.AddNode(this, 0, distance);
}

SRayIntersection CLightNode::RayNodeIntersectTest(const CRay& rkRay, uint32 AssetID, const SViewInfo& rkViewInfo)
{
    // todo: come up with a better way to share this code between CScriptNode and CLightNode
    SRayIntersection Out;
    Out.pNode = this;
    Out.ComponentIndex = AssetID;

    CTexture *pBillboard = CDrawUtil::GetLightTexture(mpLight->Type());

    if (!pBillboard)
    {
        Out.Hit = false;
        return Out;
    }

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
            const float TexelAlpha = pBillboard->ReadTexelAlpha(TexCoord);

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

    return Out;
}

CStructRef CLightNode::GetProperties() const
{
    return CStructRef(mpLight, mpLight->GetProperties());
}

void CLightNode::PropertyModified(IProperty* pProperty)
{
    CSceneNode::PropertyModified(pProperty);

    if (pProperty->Name() == "Position")
        SetPosition( mpLight->Position() );
}

CVector2f CLightNode::BillboardScale() const
{
    return AbsoluteScale().XZ() * 0.75f;
}

void CLightNode::CalculateTransform(CTransform4f& rOut) const
{
    // Billboards don't rotate and their scale is applied separately
    rOut.Translate(AbsolutePosition());
}
