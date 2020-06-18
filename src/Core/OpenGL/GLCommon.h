#ifndef GLCOMMON_H
#define GLCOMMON_H

#include <Common/BasicTypes.h>
#include <GL/glew.h>
#include <array>

enum class EBlendFactor
{
    Zero        = GL_ZERO,
    One         = GL_ONE,
    SrcColor    = GL_SRC_COLOR,
    InvSrcColor = GL_ONE_MINUS_SRC_COLOR,
    SrcAlpha    = GL_SRC_ALPHA,
    InvSrcAlpha = GL_ONE_MINUS_SRC_ALPHA,
    DstAlpha    = GL_DST_ALPHA,
    InvDstAlpha = GL_ONE_MINUS_DST_ALPHA
};

enum class EPrimitiveType
{
    // The values assigned here match the defines for primitive types in GX
    // and appear in geometry data in game file formats
    Quads           = 0x80,
    Triangles       = 0x90,
    TriangleStrip   = 0x98,
    TriangleFan     = 0xA0,
    Lines           = 0xA8,
    LineStrip       = 0xB0,
    Points          = 0xB8
};

extern const std::array<GLenum, 8> gBlendFactor;
extern const std::array<GLenum, 7> gZMode;
GLenum GXPrimToGLPrim(EPrimitiveType Type);

#endif // GLCOMMON_H
