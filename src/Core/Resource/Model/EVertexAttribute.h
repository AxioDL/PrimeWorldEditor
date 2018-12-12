#ifndef EVERTEXATTRIBUTE
#define EVERTEXATTRIBUTE

#include <Common/Flags.h>

enum EVertexAttribute
{
    eNoAttributes   = 0x0,
    ePosition       = 0x1,
    eNormal         = 0x2,
    eColor0         = 0x4,
    eColor1         = 0x8,
    eTex0           = 0x10,
    eTex1           = 0x20,
    eTex2           = 0x40,
    eTex3           = 0x80,
    eTex4           = 0x100,
    eTex5           = 0x200,
    eTex6           = 0x400,
    eTex7           = 0x800,
    eBoneIndices    = 0x1000,
    eBoneWeights    = 0x2000,
    ePosMtx         = 0x4000,
    eTex0Mtx        = 0x8000,
    eTex1Mtx        = 0x10000,
    eTex2Mtx        = 0x20000,
    eTex3Mtx        = 0x40000,
    eTex4Mtx        = 0x80000,
    eTex5Mtx        = 0x100000,
    eTex6Mtx        = 0x200000
};
DECLARE_FLAGS(EVertexAttribute, FVertexDescription)

extern const uint32 gkNumVertexAttribs;
uint32 VertexAttributeSize(EVertexAttribute Attrib);

#endif // EVERTEXATTRIBUTE

