#include "EVertexAttribute.h"
#include <Common/Log.h>

const u32 gkNumVertexAttribs = 22;

u32 VertexAttributeSize(EVertexAttribute Attrib)
{
    switch (Attrib)
    {
    case ePosition:
    case eNormal:
        return 0x0C;
    case eColor0:
    case eColor1:
    case eBoneWeights:
        return 0x10;
    case eTex0:
    case eTex1:
    case eTex2:
    case eTex3:
    case eTex4:
    case eTex5:
    case eTex6:
    case eTex7:
        return 0x08;
    case eBoneIndices:
        return 0x04;
    default:
        Log::Error("AttributeSize(): Unknown vertex attribute: " + TString::FromInt32(Attrib, 0, 10));
        return 0x00;
    }
}
