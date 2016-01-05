#ifndef EVERTEXATTRIBUTE
#define EVERTEXATTRIBUTE

#include <Common/Flags.h>

enum EVertexAttribute
{
    eNoAttributes  = 0x0,
    ePosition = 0x3,
    eNormal   = 0xC,
    eColor0   = 0x30,
    eColor1   = 0xC0,
    eTex0     = 0x300,
    eTex1     = 0xC00,
    eTex2     = 0x3000,
    eTex3     = 0xC000,
    eTex4     = 0x30000,
    eTex5     = 0xC0000,
    eTex6     = 0x300000,
    eTex7     = 0xC00000,
    ePosMtx   = 0x1000000,
    eTex0Mtx  = 0x2000000,
    eTex1Mtx  = 0x4000000,
    eTex2Mtx  = 0x8000000,
    eTex3Mtx  = 0x10000000,
    eTex4Mtx  = 0x20000000,
    eTex5Mtx  = 0x40000000,
    eTex6Mtx  = 0x80000000
};
DECLARE_FLAGS(EVertexAttribute, FVertexDescription)

#endif // EVERTEXATTRIBUTE

