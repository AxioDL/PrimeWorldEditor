#ifndef CGL_H
#define CGL_H

#include "GLCommon.h"
#include <Common/types.h>
#include <GL/glew.h>

class CGL
{
public:
    void SetBlendMode(EBlendFactor Source, EBlendFactor Dest);
    void SetOpaqueBlend();
    void SetAlphaBlend();
    void SetAdditiveBlend();

private:
    static void Init();

    static bool mInitialized;
    static EBlendFactor mBlendSrcFac, mBlendDstFac;
    static u8 mColorMask;
    static bool mDepthMask;
    static bool mStencilMask;
};

#endif // CGL_H
