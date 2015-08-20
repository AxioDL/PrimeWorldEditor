#ifndef CGIZMO_H
#define CGIZMO_H

#include <Common/CPlane.h>
#include <Common/CQuaternion.h>
#include <Common/CVector3f.h>
#include <Common/EnumUtil.h>
#include <Core/CCamera.h>
#include <Core/CToken.h>
#include <Core/IRenderable.h>
#include <Resource/model/CModel.h>

#define CGIZMO_TRANSLATE_X 0
#define CGIZMO_TRANSLATE_Y 1
#define CGIZMO_TRANSLATE_Z 2
#define CGIZMO_TRANSLATE_LINES_XY 3
#define CGIZMO_TRANSLATE_LINES_XZ 4
#define CGIZMO_TRANSLATE_LINES_YZ 5
#define CGIZMO_TRANSLATE_POLY_XY 6
#define CGIZMO_TRANSLATE_POLY_XZ 7
#define CGIZMO_TRANSLATE_POLY_YZ 8
#define CGIZMO_ROTATE_X 0
#define CGIZMO_ROTATE_Y 1
#define CGIZMO_ROTATE_Z 2
#define CGIZMO_ROTATE_XYZ 3
#define CGIZMO_SCALE_X 0
#define CGIZMO_SCALE_Y 1
#define CGIZMO_SCALE_Z 2
#define CGIZMO_SCALE_LINES_XY 3
#define CGIZMO_SCALE_LINES_XZ 4
#define CGIZMO_SCALE_LINES_YZ 5
#define CGIZMO_SCALE_POLY_XY 6
#define CGIZMO_SCALE_POLY_XZ 7
#define CGIZMO_SCALE_POLY_YZ 8
#define CGIZMO_SCALE_XYZ 9

class CGizmo : public IRenderable
{
public:
    enum EGizmoMode
    {
        eTranslate, eRotate, eScale
    };

    enum EGizmoAxes
    {
        eNone = 0x0,
        eX    = 0x1,
        eY    = 0x2,
        eZ    = 0x4,
        eXY   = eX | eY,
        eXZ   = eX | eZ,
        eYZ   = eY | eZ,
        eXYZ  = eX | eY | eZ
    };

private:
    EGizmoMode mMode;
    EGizmoAxes mSelectedAxes;
    CTransform4f mBillboardTransform;
    CQuaternion mBillboardRotation;
    float mGizmoSize;
    float mCameraDist;

    CTransform4f mTransform;
    CVector3f mPosition;
    CVector3f mDeltaTranslation;
    CVector3f mTotalTranslation;
    CQuaternion mRotation;
    CQuaternion mDeltaRotation;
    CQuaternion mTotalRotation;
    CVector3f mScale;
    CVector3f mDeltaScale;
    CVector3f mTotalScale;
    bool mFlipScaleX;
    bool mFlipScaleY;
    bool mFlipScaleZ;

    CPlane mTranslatePlane;
    CVector3f mLastTranslatePosition;
    CVector3f mTranslateOffset;
    bool mSetOffset;


    struct SModelPart
    {
        EGizmoAxes modelAxes;
        bool enableRayCast;
        CModel *pModel;
        CToken modelToken;

        SModelPart() {}
        SModelPart(EGizmoAxes axes, bool rayCastOn, CModel *_pModel) :
            modelAxes(axes), enableRayCast(rayCastOn), pModel(_pModel), modelToken(_pModel) {}
    };
    SModelPart *mpCurrentParts;
    u32 mNumCurrentParts;

    // Static
    static bool smModelsLoaded;
    static SModelPart smTranslateModels[9];
    static SModelPart smRotateModels[4];
    static SModelPart smScaleModels[10];
    static SModelPart smRotateClipOutline;

public:
    CGizmo();
    ~CGizmo();

    void AddToRenderer(CRenderer *pRenderer);
    void DrawAsset(ERenderOptions options, u32 asset);
    void DrawRotationOutline();

    void IncrementSize();
    void DecrementSize();
    void UpdateForCamera(const CCamera& camera);
    bool CheckSelectedAxes(const CRay& ray);
    u32 NumSelectedAxes();
    void ResetSelectedAxes();
    void StartTransform();
    bool TransformFromInput(const CRay& ray, const CCamera& camera);
    void EndTransform();

    EGizmoMode Mode();
    CVector3f Position();
    CVector3f DeltaTranslation();
    CVector3f TotalTranslation();
    void SetMode(EGizmoMode mode);
    void SetPosition(const CVector3f& position);
    void SetOrientation(const CQuaternion& orientation);

    // Protected
protected:
    void UpdateTransform();

    // Private Static
private:
    static void LoadModels();
};
DEFINE_ENUM_FLAGS(CGizmo::EGizmoAxes)

#endif // CGIZMO_H
