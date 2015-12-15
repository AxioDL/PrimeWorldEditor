#include "CGL.h"

// ************ PUBLIC ************
void CGL::SetBlendMode(EBlendFactor Source, EBlendFactor Dest)
{
    glBlendFuncSeparate(Source, Dest, eBlendZero, eBlendZero);
    mBlendSrcFac = Source;
    mBlendDstFac = Dest;
}

void CGL::SetOpaqueBlend()
{
    SetBlendMode(eBlendOne, eBlendZero);
}

void CGL::SetAlphaBlend()
{
    SetBlendMode(eBlendSrcAlpha, eBlendInvSrcAlpha);
}

void CGL::SetAdditiveBlend()
{
    SetBlendMode(eBlendOne, eBlendOne);
}

// ************ PRIVATE ************
void CGL::Init()
{
    if (!mInitialized)
    {
        glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ZERO, GL_ZERO);
        mBlendSrcFac = eBlendOne;
        mBlendDstFac = eBlendZero;
        mInitialized = true;
    }
}

// ************ STATIC MEMBER INITIALIZATION ************
bool CGL::mInitialized;
EBlendFactor CGL::mBlendSrcFac;
EBlendFactor CGL::mBlendDstFac;
