#include "CLightNode.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CGraphics.h"
#include "Core/Render/CRenderer.h"
#include <Common/Math/MathUtil.h>

CLightNode::CLightNode(CScene *pScene, uint32 NodeID, CSceneNode *pParent, CLight *pLight)
    : CSceneNode(pScene, NodeID, pParent)
    , mpLight(pLight)
{
    mLocalAABox = CAABox::skOne;
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
        CAABox RadiusBox = (CAABox::skOne * 2.f * mpLight->GetRadius()) + mPosition;

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
    CVector2f BillScale = BillboardScale();
    float ScaleXY = (BillScale.X > BillScale.Y ? BillScale.X : BillScale.Y);

    CAABox BillBox = CAABox(mPosition + CVector3f(-ScaleXY, -ScaleXY, -BillScale.Y),
                            mPosition + CVector3f( ScaleXY,  ScaleXY,  BillScale.Y));

    std::pair<bool,float> BoxResult = BillBox.IntersectsRay(rTester.Ray());
    if (BoxResult.first) rTester.AddNode(this, 0, BoxResult.second);
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
    CPlane BillboardPlane(-rkViewInfo.pCamera->Direction(), mPosition);
    std::pair<bool,float> PlaneTest = Math::RayPlaneIntersection(rkRay, BillboardPlane);

    if (PlaneTest.first)
    {
        // Step 2: transform the hit point into the plane's local space
        CVector3f PlaneHitPoint = rkRay.PointOnRay(PlaneTest.second);
        CVector3f RelHitPoint = PlaneHitPoint - mPosition;

        CVector3f PlaneForward = -rkViewInfo.pCamera->Direction();
        CVector3f PlaneRight = -rkViewInfo.pCamera->RightVector();
        CVector3f PlaneUp = rkViewInfo.pCamera->UpVector();
        CQuaternion PlaneRot = CQuaternion::FromAxes(PlaneRight, PlaneForward, PlaneUp);

        CVector3f RotatedHitPoint = PlaneRot.Inverse() * RelHitPoint;
        CVector2f LocalHitPoint = RotatedHitPoint.XZ() / BillboardScale();

        // Step 3: check whether the transformed hit point is in the -1 to 1 range
        if ((LocalHitPoint.X >= -1.f) && (LocalHitPoint.X <= 1.f) && (LocalHitPoint.Y >= -1.f) && (LocalHitPoint.Y <= 1.f))
        {
            // Step 4: look up the hit texel and check whether it's transparent or opaque
            CVector2f TexCoord = (LocalHitPoint + CVector2f(1.f)) * 0.5f;
            TexCoord.X = -TexCoord.X + 1.f;
            float TexelAlpha = pBillboard->ReadTexelAlpha(TexCoord);

            if (TexelAlpha < 0.25f)
                Out.Hit = false;

            else
            {
                // It's opaque... we have a hit!
                Out.Hit = true;
                Out.Distance = PlaneTest.second;
            }
        }

        else
            Out.Hit = false;
    }

    else
        Out.Hit = false;

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

CLight* CLightNode::Light()
{
    return mpLight;
}

CVector2f CLightNode::BillboardScale()
{
    return AbsoluteScale().XZ() * 0.75f;
}

void CLightNode::CalculateTransform(CTransform4f& rOut) const
{
    // Billboards don't rotate and their scale is applied separately
    rOut.Translate(AbsolutePosition());
}
