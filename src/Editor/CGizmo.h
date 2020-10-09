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
#include <array>

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
    EGizmoMode mMode{EGizmoMode::Off};
    FAxes mSelectedAxes{EAxis::None};
    ETransformSpace mTransformSpace{ETransformSpace::World};
    CQuaternion mBillboardRotation;
    float mGizmoSize = 1.0f;
    float mCameraDist = 0.0f;
    bool mIsTransforming = false;
    bool mHasTransformed = false;
    CVector2f mWrapOffset{0.0f};
    bool mEnableCursorWrap = true;

    // Transform
    CTransform4f mTransform;
    CTransform4f mBillboardTransform;
    CTransform4f mScaledTransform;
    CVector3f mPosition{CVector3f::Zero()};
    CQuaternion mRotation{CQuaternion::Identity()};
    CQuaternion mLocalRotation{CQuaternion::Identity()};
    CVector3f mScale{CVector3f::One()};
    bool mFlipScaleX = false;
    bool mFlipScaleY = false;
    bool mFlipScaleZ = false;

    // Delta transform
    CVector3f mDeltaTranslation{CVector3f::Zero()};
    CVector3f mTotalTranslation;
    CQuaternion mDeltaRotation{CQuaternion::Identity()};
    CQuaternion mCurrentRotation;
    CVector3f mTotalRotation; // This is a CVector3f because this value displays on the UI and a quat would cause rollover
    CVector3f mDeltaScale{CVector3f::One()};
    CVector3f mTotalScale{CVector3f::One()};

    // Transform helpers
    CPlane mTranslatePlane;
    CVector3f mTranslateOffset;
    float mRotateOffset = 0.0f;
    float mScaleOffset = 0.0f;
    bool mSetOffset = false;
    CVector3f mHitPoint;
    CVector3f mMoveDir;

    // Model parts
    struct SModelPart
    {
        FAxes ModelAxes;
        bool EnableRayCast = false;
        bool IsBillboard = false;
        TResPtr<CModel> pModel;

        SModelPart() :
                ModelAxes(EAxis::None)
        {};

        SModelPart(FAxes Axes, bool RayCastOn, bool Billboard, TResPtr<CModel> _pModel) :
                ModelAxes(Axes), EnableRayCast(RayCastOn), IsBillboard(Billboard), pModel(_pModel)
        {}
    };

    SModelPart* mpCurrentParts = nullptr;
    uint32 mNumCurrentParts = 0;

    // Static
    static inline bool smModelsLoaded = false;
    static inline std::array<SModelPart, CGIZMO_TRANSLATE_NUM> smTranslateModels;
    static inline std::array<SModelPart, CGIZMO_ROTATE_NUM> smRotateModels;
    static inline std::array<SModelPart, CGIZMO_SCALE_NUM> smScaleModels;

public:
    CGizmo();
    ~CGizmo() override;

    void AddToRenderer(CRenderer* pRenderer, const SViewInfo& rkViewInfo) override;
    void Draw(FRenderOptions Options, int ComponentIndex, ERenderCommand Command, const SViewInfo& rkViewInfo) override;

    void IncrementSize();
    void DecrementSize();
    void UpdateForCamera(const CCamera& rkCamera);
    bool CheckSelectedAxes(const CRay& rkRay);
    uint32 NumSelectedAxes() const;
    void ResetSelectedAxes();
    void StartTransform();
    bool TransformFromInput(const CRay& rkRay, CCamera& rkCamera);
    void EndTransform();

    // Accessors
    EGizmoMode Mode() const
    { return mMode; }

    ETransformSpace TransformSpace() const
    { return mTransformSpace; }

    CVector3f Position() const
    { return mPosition; }

    CVector3f DeltaTranslation() const
    { return mDeltaTranslation; }

    CVector3f TotalTranslation() const
    { return mTotalTranslation; }

    CQuaternion Rotation() const
    { return mRotation; }

    CQuaternion DeltaRotation() const
    { return mDeltaRotation; }

    CVector3f TotalRotation() const
    { return mTotalRotation; }

    CVector3f Scale() const
    { return mScale; }

    CVector3f DeltaScale() const
    { return mDeltaScale; }

    CVector3f TotalScale() const
    { return mTotalScale; }

    bool IsTransforming() const
    { return mIsTransforming; }

    bool HasTransformed() const
    { return mHasTransformed; }

    void SetPosition(const CVector3f& rkPosition)
    { mPosition = rkPosition; }

    void EnableCursorWrap(bool Enable)
    { mEnableCursorWrap = Enable; }

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
