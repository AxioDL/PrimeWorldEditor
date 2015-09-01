#ifndef CGIZMO_H
#define CGIZMO_H

#include <Common/CPlane.h>
#include <Common/CQuaternion.h>
#include <Common/CVector3f.h>
#include <Common/EnumUtil.h>
#include <Common/ETransformSpace.h>
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
#define CGIZMO_ROTATE_OUTLINE 0
#define CGIZMO_ROTATE_X 1
#define CGIZMO_ROTATE_Y 2
#define CGIZMO_ROTATE_Z 3
#define CGIZMO_ROTATE_XYZ 4
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
        eTranslate, eRotate, eScale, eOff
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
    ETransformSpace mTransformSpace;
    CQuaternion mBillboardRotation;
    float mGizmoSize;
    float mCameraDist;
    bool mIsTransforming;
    bool mHasTransformed;
    CVector2f mWrapOffset;
    bool mEnableCursorWrap;

    // Transform
    CTransform4f mTransform;
    CTransform4f mBillboardTransform;
    CTransform4f mScaledTransform;
    CVector3f mPosition;
    CQuaternion mRotation;
    CQuaternion mLocalRotation;
    CVector3f mScale;
    bool mFlipScaleX;
    bool mFlipScaleY;
    bool mFlipScaleZ;

    // Delta transform
    CVector3f mDeltaTranslation;
    CVector3f mTotalTranslation;
    CQuaternion mDeltaRotation;
    CQuaternion mCurrentRotation;
    CVector3f mTotalRotation; // This is a CVector3f because this value displays on the UI and a quat would cause rollover
    CVector3f mDeltaScale;
    CVector3f mTotalScale;

    // Transform helpers
    CPlane mTranslatePlane;
    CVector3f mTranslateOffset;
    float mRotateOffset;
    float mScaleOffset;
    bool mSetOffset;
    CVector3f mHitPoint;
    CVector3f mMoveDir;

    // Model parts
    struct SModelPart
    {
        EGizmoAxes modelAxes;
        bool enableRayCast;
        bool isBillboard;
        CModel *pModel;
        CToken modelToken;

        SModelPart() {}
        SModelPart(EGizmoAxes axes, bool rayCastOn, bool billboard, CModel *_pModel) :
            modelAxes(axes), enableRayCast(rayCastOn), isBillboard(billboard), pModel(_pModel), modelToken(_pModel) {}
    };
    SModelPart *mpCurrentParts;
    u32 mNumCurrentParts;

    // Static
    static bool smModelsLoaded;
    static SModelPart smTranslateModels[9];
    static SModelPart smRotateModels[5];
    static SModelPart smScaleModels[10];

public:
    CGizmo();
    ~CGizmo();

    void AddToRenderer(CRenderer *pRenderer);
    void DrawAsset(ERenderOptions options, u32 asset);

    void IncrementSize();
    void DecrementSize();
    void UpdateForCamera(const CCamera& camera);
    bool CheckSelectedAxes(const CRay& ray);
    u32 NumSelectedAxes();
    void ResetSelectedAxes();
    void StartTransform();
    bool TransformFromInput(const CRay& ray, CCamera& camera);
    void EndTransform();
    bool IsTransforming();
    bool HasTransformed();

    EGizmoMode Mode();
    ETransformSpace TransformSpace();
    CVector3f Position();
    CVector3f DeltaTranslation();
    CVector3f TotalTranslation();
    CQuaternion Rotation();
    CQuaternion DeltaRotation();
    CVector3f TotalRotation();
    CVector3f Scale();
    CVector3f DeltaScale();
    CVector3f TotalScale();
    void SetMode(EGizmoMode mode);
    void SetTransformSpace(ETransformSpace space);
    void SetPosition(const CVector3f& position);
    void SetLocalRotation(const CQuaternion& orientation);
    void EnableCursorWrap(bool wrap);

    // Protected
protected:
    void UpdateTransform();
    void WrapCursor();

    // Private Static
private:
    static void LoadModels();
};
DEFINE_ENUM_FLAGS(CGizmo::EGizmoAxes)

#endif // CGIZMO_H
