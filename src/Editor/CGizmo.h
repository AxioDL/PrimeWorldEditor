#ifndef CGIZMO_H
#define CGIZMO_H

#include <Common/Flags.h>
#include <Common/Math/CPlane.h>
#include <Common/Math/CQuaternion.h>
#include <Common/Math/CVector3f.h>
#include <Common/Math/EAxis.h>
#include <Common/Math/ETransformSpace.h>
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
    enum class EGizmoMode
    {
        Translate, Rotate, Scale, Off
    };

private:
    EGizmoMode mMode;
    FAxes mSelectedAxes;
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
        FAxes ModelAxes;
        bool EnableRayCast;
        bool IsBillboard;
        TResPtr<CModel> pModel;

        SModelPart() {}
        SModelPart(FAxes Axes, bool RayCastOn, bool Billboard, TResPtr<CModel> _pModel) :
            ModelAxes(Axes), EnableRayCast(RayCastOn), IsBillboard(Billboard), pModel(_pModel) {}
    };
    SModelPart *mpCurrentParts;
    uint32 mNumCurrentParts;

    // Static
    static bool smModelsLoaded;
    static SModelPart smTranslateModels[CGIZMO_TRANSLATE_NUM];
    static SModelPart smRotateModels[CGIZMO_ROTATE_NUM];
    static SModelPart smScaleModels[CGIZMO_SCALE_NUM];

public:
    CGizmo();
    ~CGizmo();

    void AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo);
    void Draw(FRenderOptions Options, int ComponentIndex, ERenderCommand Command, const SViewInfo& rkViewInfo);

    void IncrementSize();
    void DecrementSize();
    void UpdateForCamera(const CCamera& rkCamera);
    bool CheckSelectedAxes(const CRay& rkRay);
    uint32 NumSelectedAxes();
    void ResetSelectedAxes();
    void StartTransform();
    bool TransformFromInput(const CRay& rkRay, CCamera& rkCamera);
    void EndTransform();

    // Accessors
    inline EGizmoMode Mode() const                  { return mMode; }
    inline ETransformSpace TransformSpace() const   { return mTransformSpace; }
    inline CVector3f Position() const               { return mPosition; }
    inline CVector3f DeltaTranslation() const       { return mDeltaTranslation; }
    inline CVector3f TotalTranslation() const       { return mTotalTranslation; }
    inline CQuaternion Rotation() const             { return mRotation; }
    inline CQuaternion DeltaRotation() const        { return mDeltaRotation; }
    inline CVector3f TotalRotation() const          { return mTotalRotation; }
    inline CVector3f Scale() const                  { return mScale; }
    inline CVector3f DeltaScale() const             { return mDeltaScale; }
    inline CVector3f TotalScale() const             { return mTotalScale; }
    inline bool IsTransforming() const              { return mIsTransforming; }
    inline bool HasTransformed() const              { return mHasTransformed; }

    inline void SetPosition(const CVector3f& rkPosition)    { mPosition = rkPosition; }
    inline void EnableCursorWrap(bool Enable)               { mEnableCursorWrap = Enable; }
    void SetMode(EGizmoMode mode);
    void SetTransformSpace(ETransformSpace Space);
    void SetLocalRotation(const CQuaternion& rkOrientation);

    // Protected
protected:
    void UpdateTransform();
    void WrapCursor();

    // Private Static
private:
    static void LoadModels();
};

#endif // CGIZMO_H
