#ifndef CGIZMO_H
#define CGIZMO_H

#include <Common/Flags.h>
#include <Math/CPlane.h>
#include <Math/CQuaternion.h>
#include <Math/CVector3f.h>
#include <Math/ETransformSpace.h>
#include <Core/Render/CCamera.h>
#include <Core/Render/IRenderable.h>
#include <Core/Resource/Model/CModel.h>
#include <Core/Resource/TResPtr.h>

#define CGIZMO_TRANSLATE_X 0
#define CGIZMO_TRANSLATE_Y 1
#define CGIZMO_TRANSLATE_Z 2
#define CGIZMO_TRANSLATE_LINES_XY 3
#define CGIZMO_TRANSLATE_LINES_XZ 4
#define CGIZMO_TRANSLATE_LINES_YZ 5
#define CGIZMO_TRANSLATE_POLY_XY 6
#define CGIZMO_TRANSLATE_POLY_XZ 7
#define CGIZMO_TRANSLATE_POLY_YZ 8
#define CGIZMO_TRANSLATE_NUM 9
#define CGIZMO_ROTATE_OUTLINE 0
#define CGIZMO_ROTATE_X 1
#define CGIZMO_ROTATE_Y 2
#define CGIZMO_ROTATE_Z 3
#define CGIZMO_ROTATE_XYZ 4
#define CGIZMO_ROTATE_NUM 5
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
#define CGIZMO_SCALE_NUM 10

class CGizmo : public IRenderable
{
public:
    enum EGizmoMode
    {
        eTranslate, eRotate, eScale, eOff
    };

    enum EGizmoAxis
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
    DECLARE_FLAGS(EGizmoAxis, FGizmoAxes)

private:
    EGizmoMode mMode;
    FGizmoAxes mSelectedAxes;
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
        FGizmoAxes modelAxes;
        bool enableRayCast;
        bool isBillboard;
        TResPtr<CModel> pModel;

        SModelPart() {}
        SModelPart(FGizmoAxes axes, bool rayCastOn, bool billboard, TResPtr<CModel> _pModel) :
            modelAxes(axes), enableRayCast(rayCastOn), isBillboard(billboard), pModel(_pModel) {}
    };
    SModelPart *mpCurrentParts;
    u32 mNumCurrentParts;

    // Static
    static bool smModelsLoaded;
    static SModelPart smTranslateModels[CGIZMO_TRANSLATE_NUM];
    static SModelPart smRotateModels[CGIZMO_ROTATE_NUM];
    static SModelPart smScaleModels[CGIZMO_SCALE_NUM];

public:
    CGizmo();
    ~CGizmo();

    void AddToRenderer(CRenderer *pRenderer, const SViewInfo& ViewInfo);
    void Draw(FRenderOptions Options, int ComponentIndex, const SViewInfo& ViewInfo);

    void IncrementSize();
    void DecrementSize();
    void UpdateForCamera(const CCamera& camera);
    bool CheckSelectedAxes(const CRay& ray);
    u32 NumSelectedAxes();
    void ResetSelectedAxes();
    void StartTransform();
    bool TransformFromInput(const CRay& ray, CCamera& camera);
    void EndTransform();
    bool IsTransforming() const;
    bool HasTransformed() const;

    EGizmoMode Mode() const;
    ETransformSpace TransformSpace() const;
    CVector3f Position() const;
    CVector3f DeltaTranslation() const;
    CVector3f TotalTranslation() const;
    CQuaternion Rotation() const;
    CQuaternion DeltaRotation() const;
    CVector3f TotalRotation() const;
    CVector3f Scale() const;
    CVector3f DeltaScale() const;
    CVector3f TotalScale() const;
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

#endif // CGIZMO_H
