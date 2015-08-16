#ifndef CGIZMO_H
#define CGIZMO_H

#include <Common/CVector3f.h>
#include <Common/CQuaternion.h>
#include <Common/EnumUtil.h>
#include <Core/CCamera.h>
#include <Core/CToken.h>
#include <Core/IRenderable.h>
#include <Resource/model/CModel.h>

#define CGIZMO_TRANSLATE_X 0
#define CGIZMO_TRANSLATE_Y 1
#define CGIZMO_TRANSLATE_Z 2
#define CGIZMO_TRANSLATE_XY 3
#define CGIZMO_TRANSLATE_XZ 4
#define CGIZMO_TRANSLATE_YZ 5
#define CGIZMO_ROTATE_X 0
#define CGIZMO_ROTATE_Y 1
#define CGIZMO_ROTATE_Z 2
#define CGIZMO_ROTATE_XYZ 3
#define CGIZMO_SCALE_X 0
#define CGIZMO_SCALE_Y 1
#define CGIZMO_SCALE_Z 2
#define CGIZMO_SCALE_XY 3
#define CGIZMO_SCALE_XZ 4
#define CGIZMO_SCALE_YZ 5
#define CGIZMO_SCALE_XYZ 6

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
    CQuaternion mRotation;
    CVector3f mScale;
    CVector3f mDeltaPosition;
    CQuaternion mDeltaRotation;
    CVector3f mDeltaScale;
    bool mFlipScaleX;
    bool mFlipScaleY;
    bool mFlipScaleZ;

    // Static
    struct SModelPart
    {
        EGizmoAxes modelAxes;
        CModel *pModel;
        CToken modelToken;

        SModelPart() {}
        SModelPart(EGizmoAxes axes, CModel *_pModel) :
            modelAxes(axes), pModel(_pModel), modelToken(_pModel) {}
    };

    static bool smModelsLoaded;
    static SModelPart smTranslateModels[6];
    static SModelPart smRotateModels[4];
    static SModelPart smScaleModels[7];
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
    bool IntersectsRay(const CRay& ray);

    EGizmoMode Mode();
    void SetMode(EGizmoMode mode);
    void SetPosition(const CVector3f& position);

    // Protected
protected:
    void UpdateTransform();

    // Private Static
private:
    static void LoadModels();
};
DEFINE_ENUM_FLAGS(CGizmo::EGizmoAxes)

#endif // CGIZMO_H
