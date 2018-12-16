#ifndef EVERTEXATTRIBUTE
#define EVERTEXATTRIBUTE

#include <Common/Flags.h>

enum class EVertexAttribute
{
    None            = 0x0,
    Position        = 0x1,
    Normal          = 0x2,
    Color0          = 0x4,
    Color1          = 0x8,
    Tex0            = 0x10,
    Tex1            = 0x20,
    Tex2            = 0x40,
    Tex3            = 0x80,
    Tex4            = 0x100,
    Tex5            = 0x200,
    Tex6            = 0x400,
    Tex7            = 0x800,
    BoneIndices     = 0x1000,
    BoneWeights     = 0x2000,
    PosMtx          = 0x4000,
    Tex0Mtx         = 0x8000,
    Tex1Mtx         = 0x10000,
    Tex2Mtx         = 0x20000,
    Tex3Mtx         = 0x40000,
    Tex4Mtx         = 0x80000,
    Tex5Mtx         = 0x100000,
    Tex6Mtx         = 0x200000
};
DECLARE_FLAGS_ENUMCLASS(EVertexAttribute, FVertexDescription)

extern const uint32 gkNumVertexAttribs;
uint32 VertexAttributeSize(EVertexAttribute Attrib);

#endif // EVERTEXATTRIBUTE

