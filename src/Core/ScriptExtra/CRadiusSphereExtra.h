#ifndef CRADIUSSPHEREEXTRA_H
#define CRADIUSSPHEREEXTRA_H

#include "CScriptExtra.h"

class CRadiusSphereExtra : public CScriptExtra
{
    // Sphere visualization for objects that have a float radius property.
    uint32 mObjectType;
    CFloatRef mRadius;

public:
    explicit CRadiusSphereExtra(CScriptObject* pInstance, CScene* pScene, CScriptNode* pParent = nullptr);
    void AddToRenderer(CRenderer* pRenderer, const SViewInfo& rkViewInfo) override;
    void Draw(FRenderOptions Options, int ComponentIndex, ERenderCommand Command, const SViewInfo& rkViewInfo) override;
    CColor Color() const;
    CAABox Bounds() const;
};

#endif // CRADIUSSPHEREEXTRA_H
