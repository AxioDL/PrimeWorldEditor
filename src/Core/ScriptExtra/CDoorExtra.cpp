#include "CDoorExtra.h"
#include "Core/Render/CRenderer.h"

CDoorExtra::CDoorExtra(CScriptObject *pInstance, CSceneManager *pScene, CSceneNode *pParent)
    : CScriptExtra(pInstance, pScene, pParent)
    , mpShieldModelProp(nullptr)
    , mpShieldColorProp(nullptr)
    , mpShieldModel(nullptr)
{
    CPropertyStruct *pBaseStruct = pInstance->Properties();

    mpShieldModelProp = (CFileProperty*) pBaseStruct->PropertyByID(0xB20CC271);
    if (mpShieldModelProp && (mpShieldModelProp->Type() != eFileProperty))
        mpShieldModelProp = nullptr;

    mpShieldColorProp = (CColorProperty*) pBaseStruct->PropertyByID(0x47B4E863);
    if (mpShieldColorProp && (mpShieldColorProp->Type() != eColorProperty))
        mpShieldColorProp = nullptr;

    if (mpShieldModelProp)
        PropertyModified(mpShieldModelProp);
}

void CDoorExtra::PropertyModified(CPropertyBase *pProperty)
{
    if (pProperty == mpShieldModelProp)
    {
        mpShieldModel = mpShieldModelProp->Get();

        if (mpShieldModel)
            mLocalAABox = mpShieldModel->AABox();

        else
            mLocalAABox = CAABox::skInfinite;

        MarkTransformChanged();
    }
}

void CDoorExtra::AddToRenderer(CRenderer *pRenderer, const SViewInfo& ViewInfo)
{
    if (!mpShieldModel) return;
    if (ViewInfo.GameMode && !mpInstance->IsActive()) return;

    if (mpParent->IsVisible() && ViewInfo.ViewFrustum.BoxInFrustum(AABox()))
    {
        if (mpShieldModel->HasTransparency(0))
            AddSurfacesToRenderer(pRenderer, mpShieldModel, 0, ViewInfo);
        else
            pRenderer->AddOpaqueMesh(this, -1, AABox(), eDrawMesh);

        if (mpParent->IsSelected() && !ViewInfo.GameMode)
            pRenderer->AddOpaqueMesh(this, -1, AABox(), eDrawSelection);
    }
}

void CDoorExtra::Draw(ERenderOptions Options, int ComponentIndex, const SViewInfo& ViewInfo)
{
    LoadModelMatrix();
    mpParent->LoadLights(ViewInfo);

    CGraphics::SetupAmbientColor();
    CGraphics::UpdateVertexBlock();

    CColor Tint = mpParent->TintColor(ViewInfo);

    if (mpShieldColorProp)
        Tint *= mpShieldColorProp->Get();

    CGraphics::sPixelBlock.TintColor = Tint;
    CGraphics::sPixelBlock.TevColor = CColor::skWhite;
    CGraphics::UpdatePixelBlock();

    if (ComponentIndex < 0)
        mpShieldModel->Draw(Options, 0);
    else
        mpShieldModel->DrawSurface(Options, ComponentIndex, 0);
}

void CDoorExtra::DrawSelection()
{
    LoadModelMatrix();
    glBlendFunc(GL_ONE, GL_ZERO);
    mpShieldModel->DrawWireframe(eNoRenderOptions, mpParent->WireframeColor());
}

void CDoorExtra::RayAABoxIntersectTest(CRayCollisionTester& Tester, const SViewInfo& ViewInfo)
{
    if (!mpShieldModel) return;
    if (ViewInfo.GameMode && !mpInstance->IsActive()) return;

    const CRay& Ray = Tester.Ray();
    std::pair<bool,float> BoxResult = AABox().IntersectsRay(Ray);

    if (BoxResult.first)
        Tester.AddNodeModel(this, mpShieldModel);
}

SRayIntersection CDoorExtra::RayNodeIntersectTest(const CRay &Ray, u32 AssetID, const SViewInfo& ViewInfo)
{
    ERenderOptions Options = ViewInfo.pRenderer->RenderOptions();

    SRayIntersection out;
    out.pNode = mpParent;
    out.ComponentIndex = AssetID;

    CRay TransformedRay = Ray.Transformed(Transform().Inverse());
    std::pair<bool,float> Result = mpShieldModel->GetSurface(AssetID)->IntersectsRay(TransformedRay, ((Options & eEnableBackfaceCull) == 0));

    if (Result.first)
    {
        out.Hit = true;
        CVector3f HitPoint = TransformedRay.PointOnRay(Result.second);
        CVector3f WorldHitPoint = Transform() * HitPoint;
        out.Distance = Ray.Origin().Distance(WorldHitPoint);
    }

    else out.Hit = false;

    return out;
}
