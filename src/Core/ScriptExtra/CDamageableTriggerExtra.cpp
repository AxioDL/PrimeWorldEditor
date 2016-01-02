#include "CDamageableTriggerExtra.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CRenderer.h"

CDamageableTriggerExtra::CDamageableTriggerExtra(CScriptObject *pInstance, CSceneManager *pScene, CSceneNode *pParent)
    : CScriptExtra(pInstance, pScene, pParent)
    , mpRenderSideProp(nullptr)
    , mpMat(nullptr)
{
    for (u32 iTex = 0; iTex < 3; iTex++)
        mpTextures[iTex] = nullptr;

    SetInheritance(true, false, false);
    CreateMaterial();

    CPropertyStruct *pBaseStruct = pInstance->Properties();

    // Fetch render side
    mpRenderSideProp = (TEnumProperty*) pBaseStruct->PropertyByIndex(0x5);

    if (mpRenderSideProp && mpRenderSideProp->Type() != eEnumProperty)
        mpRenderSideProp = nullptr;

    if (mpRenderSideProp) PropertyModified(mpRenderSideProp);

    // Fetch scale
    mpSizeProp = (TVector3Property*) pBaseStruct->PropertyByIndex(0x2);

    if (mpSizeProp && mpSizeProp->Type() != eVector3Property)
        mpSizeProp = nullptr;

    if (mpSizeProp) PropertyModified (mpSizeProp);

    // Fetch textures
    for (u32 iTex = 0; iTex < 3; iTex++)
    {
        mpTextureProps[iTex] = (TFileProperty*) pBaseStruct->PropertyByIndex(0x6 + iTex);

        if (mpTextureProps[iTex])
        {
            if (mpTextureProps[iTex]->Type() == eFileProperty)
                PropertyModified(mpTextureProps[iTex]);
            else
                mpTextureProps[iTex] = nullptr;
        }

        else
            mpTextures[iTex] = nullptr;
    }
}

CDamageableTriggerExtra::~CDamageableTriggerExtra()
{
    delete mpMat;
}

void CDamageableTriggerExtra::CreateMaterial()
{
    if (mpMat) delete mpMat;
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
    CVector3f Extent = mPlaneSize / 2.f;

    switch (mRenderSide)
    {
    case eNorth:
    case eSouth:
    {
        float Scalar = (mRenderSide == eNorth ? 1.f : -1.f);

        mPosition = CVector3f(0.f, Extent.y * Scalar, 0.f);
        mRotation = CQuaternion::FromEuler(CVector3f(90.f * Scalar, 0.f, 0.f));
        mScale = CVector3f(Extent.x, Extent.z, 0.f);
        mCoordScale = mPlaneSize.xz();
        break;
    }

    case eWest:
    case eEast:
    {
        float Scalar = (mRenderSide == eWest ? 1.f : -1.f);

        mPosition = CVector3f(-Extent.x * Scalar, 0.f, 0.f);
        mRotation = CQuaternion::FromEuler(CVector3f(0.f, 90.f * Scalar, 0.f));
        mScale = CVector3f(Extent.z, Extent.y, 0.f);
        mCoordScale = -mPlaneSize.yz();
        break;
    }

    case eUp:
    case eDown:
    {
        float Scalar = (mRenderSide == eUp ? 1.f : -1.f);
        float RotAngle = (mRenderSide == eUp ? 180.f : 0.f);

        mPosition = CVector3f(0.f, 0.f, Extent.z * Scalar);
        mRotation = CQuaternion::FromEuler(CVector3f(0.f, RotAngle, 0.f));
        mScale = CVector3f(Extent.x, Extent.y, 0.f);
        mCoordScale = -mPlaneSize.xy();
        break;
    }

    }

    if (mRenderSide == eNoRender)
        mLocalAABox = CAABox::skZero;
    else
        mLocalAABox = CAABox(CVector3f(-1.f, -1.f, 0.f), CVector3f(1.f, 1.f, 0.f));

    MarkTransformChanged();
}

void CDamageableTriggerExtra::PropertyModified(IProperty *pProperty)
{
    if (pProperty == mpRenderSideProp)
    {
        mRenderSide = (ERenderSide) mpRenderSideProp->Get();
        UpdatePlaneTransform();
    }

    else if (pProperty == mpSizeProp)
    {
        mPlaneSize = mpSizeProp->Get();
        UpdatePlaneTransform();
    }

    else
    {
        for (u32 iTex = 0; iTex < 3; iTex++)
        {
            if (pProperty == mpTextureProps[iTex])
            {
                mpTextures[iTex] = mpTextureProps[iTex]->Get();

                if (mpTextures[iTex] && mpTextures[iTex]->Type() != eTexture)
                    mpTextures[iTex] = nullptr;

                mpMat->Pass(iTex)->SetTexture(mpTextures[iTex]);
                break;
            }
        }
    }
}

bool CDamageableTriggerExtra::ShouldDrawNormalAssets()
{
    return (mRenderSide == eNoRender);
}

void CDamageableTriggerExtra::AddToRenderer(CRenderer *pRenderer, const SViewInfo& ViewInfo)
{
    if (ViewInfo.GameMode && !mpInstance->IsActive())
        return;

    if (mRenderSide != eNoRender)
    {
        if (ViewInfo.ViewFrustum.BoxInFrustum(AABox()))
            pRenderer->AddTransparentMesh(this, -1, AABox(), eDrawMesh);
        if (mpParent->IsSelected() && !ViewInfo.GameMode)
            pRenderer->AddOpaqueMesh(this, -1, AABox(), eDrawSelection);
    }
}

void CDamageableTriggerExtra::Draw(ERenderOptions Options, int /*ComponentIndex*/, const SViewInfo& ViewInfo)
{
    LoadModelMatrix();
    CGraphics::sPixelBlock.TintColor = mpParent->TintColor(ViewInfo);
    mpMat->SetCurrent(Options);

    // Note: The plane the game renders this onto is 5x4.5, which is why we divide the tex coords by this value
    CVector2f TexUL(0.f, mCoordScale.y / 4.5f);
    CVector2f TexUR(mCoordScale.x / 5.f, mCoordScale.y   / 4.5f);
    CVector2f TexBR(mCoordScale.x / 5.f, 0.f);
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

void CDamageableTriggerExtra::RayAABoxIntersectTest(CRayCollisionTester& Tester, const SViewInfo& ViewInfo)
{
    if (mRenderSide == eNoRender) return;
    if (ViewInfo.GameMode && !mpInstance->IsActive()) return;

    const CRay& Ray = Tester.Ray();

    if (ViewInfo.pRenderer->RenderOptions() & eEnableBackfaceCull)
    {
        // We're guaranteed to be axis-aligned, so we can take advantage of that
        // to perform a very simple backface check.
        switch (mRenderSide)
        {
        case eNorth: if (Ray.Origin().y > AbsolutePosition().y) return; break;
        case eSouth: if (Ray.Origin().y < AbsolutePosition().y) return; break;
        case eWest:  if (Ray.Origin().x < AbsolutePosition().x) return; break;
        case eEast:  if (Ray.Origin().x > AbsolutePosition().x) return; break;
        case eUp:    if (Ray.Origin().z > AbsolutePosition().z) return; break;
        case eDown:  if (Ray.Origin().z < AbsolutePosition().z) return; break;
        }
    }

    std::pair<bool,float> Result = AABox().IntersectsRay(Ray);

    if (Result.first)
        Tester.AddNode(this, -1, Result.second);
}

SRayIntersection CDamageableTriggerExtra::RayNodeIntersectTest(const CRay& Ray, u32 /*ComponentIndex*/, const SViewInfo& /*ViewInfo*/)
{
    // The bounding box and all other tests already passed in RayAABoxIntersectTest, so we
    // already know that we have a positive. We just need the distance again.
    std::pair<bool,float> Result = AABox().IntersectsRay(Ray);
    return SRayIntersection(true, Result.second, mpParent, -1);
}
