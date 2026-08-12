#include "GLCommon.h"

#include <array>
#include <Common/Macros.h>

constexpr std::array<GLenum, 8> sBlendFactor
{
    GL_ZERO,                // GX_BL_ZERO
    GL_ONE,                 // GX_BL_ONE
    GL_SRC_COLOR,           // GX_BL_SRCCLR / GX_BL_DSTCLR
    GL_ONE_MINUS_SRC_COLOR, // GX_BL_INVSRCCLR / GX_BL_INVDSTCLR
    GL_SRC_ALPHA,           // GX_BL_SRCALPHA
    GL_ONE_MINUS_SRC_ALPHA, // GX_BL_INVSRCALPHA
    GL_DST_ALPHA,           // GX_BL_DSTALPHA
    GL_ONE_MINUS_DST_ALPHA  // GX_BL_INVDSTALPHA
};

constexpr std::array<GLenum, 7> sZMode
{
    GL_NEVER,    // GX_NEVER
    GL_LESS,     // GX_LESS
    GL_EQUAL,    // GX_EQUAL
    GL_LEQUAL,   // GX_LEQUAL
    GL_GREATER,  // GX_GREATER
    GL_NOTEQUAL, // GX_NEQUAL
    GL_ALWAYS    // GX_ALWAYS
};

GLenum GetBlendFactor(size_t index)
{
    return sBlendFactor[index];
}

GLenum GetZMode(size_t index)
{
    return sZMode[index];
}

GLenum GXPrimToGLPrim(EPrimitiveType Type)
{
    switch (Type)
    {
    // Quads, triangles, and fans are converted to strips
    case EPrimitiveType::Quads:
    case EPrimitiveType::Triangles:
    case EPrimitiveType::TriangleStrip:
    case EPrimitiveType::TriangleFan:   return GL_TRIANGLE_STRIP;
    case EPrimitiveType::Lines:         return GL_LINES;
    case EPrimitiveType::LineStrip:     return GL_LINE_STRIP;
    case EPrimitiveType::Points:        return GL_POINTS;
    }

    ASSERT(false);
    return GL_INVALID_ENUM;
}
