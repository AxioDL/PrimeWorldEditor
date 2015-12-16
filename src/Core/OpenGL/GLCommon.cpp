#include "GLCommon.h"
#include <stdexcept>

GLenum glBlendFactor[] = {
    GL_ZERO,                // GX_BL_ZERO
    GL_ONE,                 // GX_BL_ONE
    GL_SRC_COLOR,           // GX_BL_SRCCLR / GX_BL_DSTCLR
    GL_ONE_MINUS_SRC_COLOR, // GX_BL_INVSRCCLR / GX_BL_INVDSTCLR
    GL_SRC_ALPHA,           // GX_BL_SRCALPHA
    GL_ONE_MINUS_SRC_ALPHA, // GX_BL_INVSRCALPHA
    GL_DST_ALPHA,           // GX_BL_DSTALPHA
    GL_ONE_MINUS_DST_ALPHA  // GX_BL_INVDSTALPHA
};


GLenum glZMode[] = {
    GL_NEVER,    // GX_NEVER
    GL_LESS,     // GX_LESS
    GL_EQUAL,    // GX_EQUAL
    GL_LEQUAL,   // GX_LEQUAL
    GL_GREATER,  // GX_GREATER
    GL_NOTEQUAL, // GX_NEQUAL
    GL_ALWAYS    // GX_ALWAYS
};

GLenum GXPrimToGLPrim(EGXPrimitiveType t) {
    switch (t) {
        case eGX_Quads: return GL_TRIANGLE_STRIP; // Quads are converted to strips
        case eGX_Triangles: return GL_TRIANGLE_STRIP; // Triangles are converted to strips
        case eGX_TriangleStrip: return GL_TRIANGLE_STRIP;
        case eGX_TriangleFan: return GL_TRIANGLE_STRIP; // Fans are converted to strips
        case eGX_Lines: return GL_LINES;
        case eGX_LineStrip: return GL_LINE_STRIP;
        case eGX_Points: return GL_POINTS;
        default: throw std::invalid_argument("Invalid GX primitive type");
    }
}