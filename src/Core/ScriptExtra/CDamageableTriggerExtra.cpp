#include "CDamageableTriggerExtra.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CRenderer.h"
#include <Common/Macros.h>
#include <Common/Math/MathUtil.h>

CDamageableTriggerExtra::CDamageableTriggerExtra(CScriptObject *pInstance, CScene *pScene, CScriptNode *pParent)
    : CScriptExtra(pInstance, pScene, pParent)
{
    SetInheritance(true, false, false);
    CreateMaterial();

    CStructProperty* pProperties = pInstance->Template()->Properties();

    // Fetch render side
    mRenderSide = TEnumRef<ERenderSide>(pInstance->PropertyData(), pProperties->ChildByIndex(5));
    if (mRenderSide.IsValid())
        PropertyModified(mRenderSide.Property());

    // Fetch scale
    mPlaneSize = CVectorRef(pInstance->PropertyData(), pProperties->ChildByIndex(2));
    if (mPlaneSize.IsValid())
        PropertyModified(mPlaneSize.Property());

    // Fetch textures
    for (size_t TextureIdx = 0; TextureIdx < mTextureAssets.size(); TextureIdx++)
    {
        mTextureAssets[TextureIdx] = CAssetRef(pInstance->PropertyData(), pProperties->ChildByIndex(6 + TextureIdx));
        if (mTextureAssets[TextureIdx].IsValid())
            PropertyModified(mTextureAssets[TextureIdx].Property());
    }
}

CDamageableTriggerExtra::~CDamageableTriggerExtra() = default;

void CDamageableTriggerExtra::CreateMaterial()
{
    ASSERT(!mpMat);
    mpMat = std::make_unique<CMaterial>(mGame, EVertexAttribute::Position | EVertexAttribute::Normal | EVertexAttribute::Tex0);

    // Most values/TEV setup were found from the executable + from graphics debuggers
    // Animation parameters are estimates from eyeballing the values ingame
    mpMat->SetBlendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    mpMat->SetLightingEnabled(true);
    mpMat->SetOptions(EMaterialOption::Transparent);
    mpMat->SetKonst(CColor(1.f, 1.f, 1.f, 0.2f), 0);
    mpMat->SetNumPasses(3);

    CMaterialPass *pPassA = mpMat->Pass(0);
    pPassA->SetKColorSel(kKonst0_RGB);
    pPassA->SetTexCoordSource(4);
    pPassA->SetTexture(mpTextures[0]);
    pPassA->SetColorInputs(kZeroRGB, kTextureRGB, kKonstRGB, kZeroRGB);
    pPassA->SetAnimMode(EUVAnimMode::UVScroll);
    pPassA->SetAnimParam(3, -0.48f);

    CMaterialPass *pPassB = mpMat->Pass(1);
    pPassB->SetTexCoordSource(4);
    pPassB->SetTexture(mpTextures[1]);
    pPassB->SetColorInputs(kZeroRGB, kTextureRGB, kPrevRGB, kZeroRGB);
    pPassB->SetAnimMode(EUVAnimMode::UVScroll);
    pPassB->SetAnimParam(2, 0.25f);
    pPassB->SetAnimParam(3, -0.3f);

    CMaterialPass *pPassC = mpMat->Pass(2);
    pPassC->SetTexCoordSource(4);
    pPassC->SetTexture(mpTextures[2]);
    pPassC->SetRasSel(kRasColor0A0);
    pPassC->SetKAlphaSel(kKonst0_A);
    pPassC->SetColorInputs(kZeroRGB, kTextureRGB, kOneRGB, kPrevRGB);
    pPassC->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kKonstAlpha);
    pPassC->SetAnimMode(EUVAnimMode::UVScroll);
    pPassC->SetAnimParam(3, -0.16f);
}

void CDamageableTriggerExtra::UpdatePlaneTransform()
{
    const CVector3f Extent = mPlaneSize.Get() / 2.f;

    switch (mRenderSide)
    {
    case ERenderSide::North:
    case ERenderSide::South:
    {
        const float Scalar = (mRenderSide == ERenderSide::North ? 1.f : -1.f);

        mPosition = CVector3f(0.f, Extent.Y * Scalar, 0.f);
        mRotation = CQuaternion::FromEuler(CVector3f(90.f * Scalar, 0.f, 0.f));
        mScale = CVector3f(Extent.X, Extent.Z, 0.f);
        mCoordScale = mPlaneSize.Get().XZ();
        break;
    }

    case ERenderSide::West:
    case ERenderSide::East:
    {
        const float Scalar = (mRenderSide == ERenderSide::West ? 1.f : -1.f);

        mPosition = CVector3f(-Extent.X * Scalar, 0.f, 0.f);
        mRotation = CQuaternion::FromEuler(CVector3f(0.f, 90.f * Scalar, 0.f));
        mScale = CVector3f(Extent.Z, Extent.Y, 0.f);
        mCoordScale = -mPlaneSize.Get().YZ();
        break;
    }

    case ERenderSide::Up:
    case ERenderSide::Down:
    {
        const float Scalar = (mRenderSide == ERenderSide::Up ? 1.f : -1.f);
        const float RotAngle = (mRenderSide == ERenderSide::Up ? 180.f : 0.f);

        mPosition = CVector3f(0.f, 0.f, Extent.Z * Scalar);
        mRotation = CQuaternion::FromEuler(CVector3f(0.f, RotAngle, 0.f));
        mScale = CVector3f(Extent.X, Extent.Y, 0.f);
        mCoordScale = -mPlaneSize.Get().XY();
        break;
    }
    default: break;
    }

    if (mRenderSide == ERenderSide::NoRender)
        mLocalAABox = CAABox::Zero();
    else
        mLocalAABox = CAABox(CVector3f(-1.f, -1.f, 0.f), CVector3f(1.f, 1.f, 0.f));

    MarkTransformChanged();
}

CDamageableTriggerExtra::ERenderSide CDamageableTriggerExtra::RenderSideForDirection(const CVector3f& rkDir) const 
{
    // Get the index of the largest XYZ component
    const CVector3f AbsDir(Math::Abs(rkDir.X), Math::Abs(rkDir.Y), Math::Abs(rkDir.Z));
    uint32 Max = (AbsDir.X > AbsDir.Y ? 0 : 1);
    Max = (AbsDir[Max] > AbsDir.Z ? Max : 2);

    // Check whether the direction is positive or negative. If the absolute value of the component matches the input one, then it's positive.
    const bool Positive = (rkDir[Max] == AbsDir[Max]);

    // Return corresponding side for direction
    if (Max == 0) return (Positive ? ERenderSide::East : ERenderSide::West);
    if (Max == 1) return (Positive ? ERenderSide::North : ERenderSide::South);
    if (Max == 2) return (Positive ? ERenderSide::Up : ERenderSide::Down);

    return ERenderSide::NoRender;
}

CDamageableTriggerExtra::ERenderSide CDamageableTriggerExtra::TransformRenderSide(ERenderSide Side) const
{
    // DamageableTrigger has a convenience feature implemented that changes the
    // render side when the area's been rotated, so we need to replicate it here.
    const CQuaternion AreaRotation = mpScriptNode->Instance()->Area()->Transform().ExtractRotation();

    switch (Side)
    {
    case ERenderSide::North:
        return RenderSideForDirection(AreaRotation.YAxis());
    case ERenderSide::South:
        return RenderSideForDirection(-AreaRotation.YAxis());
    case ERenderSide::West:
        return RenderSideForDirection(-AreaRotation.XAxis());
    case ERenderSide::East:
        return RenderSideForDirection(AreaRotation.XAxis());
    case ERenderSide::Up:
        return RenderSideForDirection(AreaRotation.ZAxis());
    case ERenderSide::Down:
        return RenderSideForDirection(-AreaRotation.ZAxis());
    default:
        return ERenderSide::NoRender;
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
        for (uint32 TextureIdx = 0; TextureIdx < 3; TextureIdx++)
        {
            if (pProperty == mTextureAssets[TextureIdx].Property())
            {
                mpTextures[TextureIdx] = gpResourceStore->LoadResource<CTexture>( mTextureAssets[TextureIdx].Get() );

                if (mpTextures[TextureIdx] && mpTextures[TextureIdx]->Type() != EResourceType::Texture)
                    mpTextures[TextureIdx] = nullptr;

                mpMat->Pass(TextureIdx)->SetTexture(mpTextures[TextureIdx]);
                break;
            }
        }
    }
}

bool CDamageableTriggerExtra::ShouldDrawNormalAssets()
{
    return (mRenderSide == ERenderSide::NoRender);
}

void CDamageableTriggerExtra::AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo)
{
    if (rkViewInfo.GameMode && !mpInstance->IsActive())
        return;

    if (!rkViewInfo.GameMode && ((rkViewInfo.ShowFlags & EShowFlag::ObjectGeometry) == 0))
        return;

    if (mRenderSide != ERenderSide::NoRender)
    {
        if (rkViewInfo.ViewFrustum.BoxInFrustum(AABox()))
            pRenderer->AddMesh(this, -1, AABox(), true, ERenderCommand::DrawMesh);
        if (mpParent->IsSelected() && !rkViewInfo.GameMode)
            pRenderer->AddMesh(this, -1, AABox(), false, ERenderCommand::DrawSelection);
    }
}

void CDamageableTriggerExtra::Draw(FRenderOptions Options, int /*ComponentIndex*/, ERenderCommand /*Command*/, const SViewInfo& rkViewInfo)
{
    LoadModelMatrix();
    CGraphics::sPixelBlock.TintColor = mpParent->TintColor(rkViewInfo);
    mpMat->SetCurrent(Options);

    // Note: The plane the game renders this onto is 5x4.5, which is why we divide the tex coords by this value
    const CVector2f TexUL(0.f, mCoordScale.Y / 4.5f);
    const CVector2f TexUR(mCoordScale.X / 5.f, mCoordScale.Y / 4.5f);
    const CVector2f TexBR(mCoordScale.X / 5.f, 0.f);
    const CVector2f TexBL(0.f, 0.f);
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
    if (mRenderSide == ERenderSide::NoRender)
        return;

    if (rkViewInfo.GameMode && !mpInstance->IsActive())
        return;

    const CRay& rkRay = rTester.Ray();

    if ((rkViewInfo.pRenderer->RenderOptions() & ERenderOption::EnableBackfaceCull) != 0)
    {
        // We're guaranteed to be axis-aligned, so we can take advantage of that
        // to perform a very simple backface check.
        switch (mRenderSide)
        {
        case ERenderSide::North: if (rkRay.Origin().Y > AbsolutePosition().Y) return; break;
        case ERenderSide::South: if (rkRay.Origin().Y < AbsolutePosition().Y) return; break;
        case ERenderSide::West:  if (rkRay.Origin().X < AbsolutePosition().X) return; break;
        case ERenderSide::East:  if (rkRay.Origin().X > AbsolutePosition().X) return; break;
        case ERenderSide::Up:    if (rkRay.Origin().Z > AbsolutePosition().Z) return; break;
        case ERenderSide::Down:  if (rkRay.Origin().Z < AbsolutePosition().Z) return; break;
        default: break;
        }
    }

    const auto [intersects, distance] = AABox().IntersectsRay(rkRay);

    if (intersects)
    {
        rTester.AddNode(this, UINT32_MAX, distance);
        mCachedRayDistance = distance;
    }
}

SRayIntersection CDamageableTriggerExtra::RayNodeIntersectTest(const CRay& rkRay, uint32 /*ComponentIndex*/, const SViewInfo& /*rkViewInfo*/)
{
    // The bounding box and all other tests already passed in RayAABoxIntersectTest, so we
    // already know that we have a positive.
    return SRayIntersection(true, mCachedRayDistance, rkRay.PointOnRay(mCachedRayDistance), mpParent, UINT32_MAX);
}
