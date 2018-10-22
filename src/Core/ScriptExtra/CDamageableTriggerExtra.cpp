#include "CDamageableTriggerExtra.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CRenderer.h"
#include <Common/AssertMacro.h>
#include <Math/MathUtil.h>

CDamageableTriggerExtra::CDamageableTriggerExtra(CScriptObject *pInstance, CScene *pScene, CScriptNode *pParent)
    : CScriptExtra(pInstance, pScene, pParent)
    , mpMat(nullptr)
{
    for (u32 iTex = 0; iTex < 3; iTex++)
        mpTextures[iTex] = nullptr;

    SetInheritance(true, false, false);
    CreateMaterial();

    CStructProperty* pProperties = pInstance->Template()->Properties();

    // Fetch render side
    mRenderSide = TEnumRef<ERenderSide>(pInstance->PropertyData(), pProperties->ChildByIndex(5));
    if (mRenderSide.IsValid()) PropertyModified(mRenderSide.Property());

    // Fetch scale
    mPlaneSize = CVectorRef(pInstance->PropertyData(), pProperties->ChildByIndex(2));
    if (mPlaneSize.IsValid()) PropertyModified(mPlaneSize.Property());

    // Fetch textures
    for (u32 TextureIdx = 0; TextureIdx < 3; TextureIdx++)
    {
        mTextureAssets[TextureIdx] = CAssetRef(pInstance->PropertyData(), pProperties->ChildByIndex(6 + TextureIdx));
        if (mTextureAssets[TextureIdx].IsValid()) PropertyModified(mTextureAssets[TextureIdx].Property());
    }
}

CDamageableTriggerExtra::~CDamageableTriggerExtra()
{
    delete mpMat;
}

void CDamageableTriggerExtra::CreateMaterial()
{
    ASSERT(!mpMat);
    mpMat = new CMaterial(mGame, ePosition | eNormal | eTex0);

    // Most values/TEV setup were found from the executable + from graphics debuggers
    // Animation parameters are estimates from eyeballing the values ingame
    mpMat->SetBlendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    mpMat->SetLightingEnabled(true);
    mpMat->SetOptions(CMaterial::eTransparent);
    mpMat->SetKonst(CColor((float) 1.f, 1.f, 1.f, 0.2f), 0);
    mpMat->SetNumPasses(3);

    CMaterialPass *pPassA = mpMat->Pass(0);
    pPassA->SetKColorSel(eKonst0_RGB);
    pPassA->SetTexCoordSource(4);
    pPassA->SetTexture(mpTextures[0]);
    pPassA->SetColorInputs(eZeroRGB, eTextureRGB, eKonstRGB, eZeroRGB);
    pPassA->SetAnimMode(eUVScroll);
    pPassA->SetAnimParam(3, -0.48f);

    CMaterialPass *pPassB = mpMat->Pass(1);
    pPassB->SetTexCoordSource(4);
    pPassB->SetTexture(mpTextures[1]);
    pPassB->SetColorInputs(eZeroRGB, eTextureRGB, ePrevRGB, eZeroRGB);
    pPassB->SetAnimMode(eUVScroll);
    pPassB->SetAnimParam(2, 0.25f);
    pPassB->SetAnimParam(3, -0.3f);

    CMaterialPass *pPassC = mpMat->Pass(2);
    pPassC->SetTexCoordSource(4);
    pPassC->SetTexture(mpTextures[2]);
    pPassC->SetRasSel(eRasColor0A0);
    pPassC->SetKAlphaSel(eKonst0_A);
    pPassC->SetColorInputs(eZeroRGB, eTextureRGB, eOneRGB, ePrevRGB);
    pPassC->SetAlphaInputs(eZeroAlpha, eZeroAlpha, eZeroAlpha, eKonstAlpha);
    pPassC->SetAnimMode(eUVScroll);
    pPassC->SetAnimParam(3, -0.16f);
}

void CDamageableTriggerExtra::UpdatePlaneTransform()
{
    CVector3f Extent = mPlaneSize.Get() / 2.f;

    switch (mRenderSide)
    {
    case eNorth:
    case eSouth:
    {
        float Scalar = (mRenderSide == eNorth ? 1.f : -1.f);

        mPosition = CVector3f(0.f, Extent.Y * Scalar, 0.f);
        mRotation = CQuaternion::FromEuler(CVector3f(90.f * Scalar, 0.f, 0.f));
        mScale = CVector3f(Extent.X, Extent.Z, 0.f);
        mCoordScale = mPlaneSize.Get().XZ();
        break;
    }

    case eWest:
    case eEast:
    {
        float Scalar = (mRenderSide == eWest ? 1.f : -1.f);

        mPosition = CVector3f(-Extent.X * Scalar, 0.f, 0.f);
        mRotation = CQuaternion::FromEuler(CVector3f(0.f, 90.f * Scalar, 0.f));
        mScale = CVector3f(Extent.Z, Extent.Y, 0.f);
        mCoordScale = -mPlaneSize.Get().YZ();
        break;
    }

    case eUp:
    case eDown:
    {
        float Scalar = (mRenderSide == eUp ? 1.f : -1.f);
        float RotAngle = (mRenderSide == eUp ? 180.f : 0.f);

        mPosition = CVector3f(0.f, 0.f, Extent.Z * Scalar);
        mRotation = CQuaternion::FromEuler(CVector3f(0.f, RotAngle, 0.f));
        mScale = CVector3f(Extent.X, Extent.Y, 0.f);
        mCoordScale = -mPlaneSize.Get().XY();
        break;
    }

    }

    if (mRenderSide == eNoRender)
        mLocalAABox = CAABox::skZero;
    else
        mLocalAABox = CAABox(CVector3f(-1.f, -1.f, 0.f), CVector3f(1.f, 1.f, 0.f));

    MarkTransformChanged();
}

CDamageableTriggerExtra::ERenderSide CDamageableTriggerExtra::RenderSideForDirection(const CVector3f& rkDir)
{
    // Get the index of the largest XYZ component
    CVector3f AbsDir(Math::Abs(rkDir.X), Math::Abs(rkDir.Y), Math::Abs(rkDir.Z));
    u32 Max = (AbsDir.X > AbsDir.Y ? 0 : 1);
    Max = (AbsDir[Max] > AbsDir.Z ? Max : 2);

    // Check whether the direction is positive or negative. If the absolute value of the component matches the input one, then it's positive.
    bool Positive = (rkDir[Max] == AbsDir[Max]);

    // Return corresponding side for direction
    if (Max == 0)       return (Positive ? eEast : eWest);
    else if (Max == 1)  return (Positive ? eNorth : eSouth);
    else if (Max == 2)  return (Positive ? eUp : eDown);

    return eNoRender;
}

CDamageableTriggerExtra::ERenderSide CDamageableTriggerExtra::TransformRenderSide(ERenderSide Side)
{
    // DamageableTrigger has a convenience feature implemented that changes the
    // render side when the area's been rotated, so we need to replicate it here.
    CQuaternion AreaRotation = mpScriptNode->Instance()->Area()->Transform().ExtractRotation();

    switch (Side)
    {
    case eNorth:
        return RenderSideForDirection(AreaRotation.YAxis());
    case eSouth:
        return RenderSideForDirection(-AreaRotation.YAxis());
    case eWest:
        return RenderSideForDirection(-AreaRotation.XAxis());
    case eEast:
        return RenderSideForDirection(AreaRotation.XAxis());
    case eUp:
        return RenderSideForDirection(AreaRotation.ZAxis());
    case eDown:
        return RenderSideForDirection(-AreaRotation.ZAxis());
    default:
        return eNoRender;
    }
}

void CDamageableTriggerExtra::OnTransformed()
{
    UpdatePlaneTransform();
}

void CDamageableTriggerExtra::PropertyModified(IProperty* pProperty)
{
    if (pProperty == mRenderSide || pProperty == mPlaneSize)
    {
        UpdatePlaneTransform();
    }

    else
    {
        for (u32 TextureIdx = 0; TextureIdx < 3; TextureIdx++)
        {
            if (pProperty == mTextureAssets[TextureIdx].Property())
            {
                mpTextures[TextureIdx] = gpResourceStore->LoadResource<CTexture>( mTextureAssets[TextureIdx].Get() );

                if (mpTextures[TextureIdx] && mpTextures[TextureIdx]->Type() != eTexture)
                    mpTextures[TextureIdx] = nullptr;

                mpMat->Pass(TextureIdx)->SetTexture(mpTextures[TextureIdx]);
                break;
            }
        }
    }
}

bool CDamageableTriggerExtra::ShouldDrawNormalAssets()
{
    return (mRenderSide == eNoRender);
}

void CDamageableTriggerExtra::AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo)
{
    if (rkViewInfo.GameMode && !mpInstance->IsActive())
        return;

    if (!rkViewInfo.GameMode && ((rkViewInfo.ShowFlags & eShowObjectGeometry) == 0))
        return;

    if (mRenderSide != eNoRender)
    {
        if (rkViewInfo.ViewFrustum.BoxInFrustum(AABox()))
            pRenderer->AddMesh(this, -1, AABox(), true, eDrawMesh);
        if (mpParent->IsSelected() && !rkViewInfo.GameMode)
            pRenderer->AddMesh(this, -1, AABox(), false, eDrawSelection);
    }
}

void CDamageableTriggerExtra::Draw(FRenderOptions Options, int /*ComponentIndex*/, ERenderCommand /*Command*/, const SViewInfo& rkViewInfo)
{
    LoadModelMatrix();
    CGraphics::sPixelBlock.TintColor = mpParent->TintColor(rkViewInfo);
    mpMat->SetCurrent(Options);

    // Note: The plane the game renders this onto is 5x4.5, which is why we divide the tex coords by this value
    CVector2f TexUL(0.f, mCoordScale.Y / 4.5f);
    CVector2f TexUR(mCoordScale.X / 5.f, mCoordScale.Y / 4.5f);
    CVector2f TexBR(mCoordScale.X / 5.f, 0.f);
    CVector2f TexBL(0.f, 0.f);
    CDrawUtil::DrawSquare(TexUL, TexUR, TexBR, TexBL);
}

void CDamageableTriggerExtra::DrawSelection()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBlendFunc(GL_ONE, GL_ZERO);
    LoadModelMatrix();
    CDrawUtil::UseColorShader(WireframeColor());
    CDrawUtil::DrawSquare();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void CDamageableTriggerExtra::RayAABoxIntersectTest(CRayCollisionTester& rTester, const SViewInfo& rkViewInfo)
{
    if (mRenderSide == eNoRender) return;
    if (rkViewInfo.GameMode && !mpInstance->IsActive()) return;

    const CRay& rkRay = rTester.Ray();

    if (rkViewInfo.pRenderer->RenderOptions() & eEnableBackfaceCull)
    {
        // We're guaranteed to be axis-aligned, so we can take advantage of that
        // to perform a very simple backface check.
        switch (mRenderSide)
        {
        case eNorth: if (rkRay.Origin().Y > AbsolutePosition().Y) return; break;
        case eSouth: if (rkRay.Origin().Y < AbsolutePosition().Y) return; break;
        case eWest:  if (rkRay.Origin().X < AbsolutePosition().X) return; break;
        case eEast:  if (rkRay.Origin().X > AbsolutePosition().X) return; break;
        case eUp:    if (rkRay.Origin().Z > AbsolutePosition().Z) return; break;
        case eDown:  if (rkRay.Origin().Z < AbsolutePosition().Z) return; break;
        }
    }

    std::pair<bool,float> Result = AABox().IntersectsRay(rkRay);

    if (Result.first)
    {
        rTester.AddNode(this, -1, Result.second);
        mCachedRayDistance = Result.second;
    }
}

SRayIntersection CDamageableTriggerExtra::RayNodeIntersectTest(const CRay& rkRay, u32 /*ComponentIndex*/, const SViewInfo& /*rkViewInfo*/)
{
    // The bounding box and all other tests already passed in RayAABoxIntersectTest, so we
    // already know that we have a positive.
    return SRayIntersection(true, mCachedRayDistance, rkRay.PointOnRay(mCachedRayDistance), mpParent, -1);
}
