#include "EVertexAttribute.h"
#include <Common/Log.h>

const uint32 gkNumVertexAttribs = 22;

uint32 VertexAttributeSize(EVertexAttribute Attrib)
{
    switch (Attrib)
    {
    case EVertexAttribute::Position:
    case EVertexAttribute::Normal:
        return 0x0C;
    case EVertexAttribute::Color0:
    case EVertexAttribute::Color1:
    case EVertexAttribute::BoneWeights:
        return 0x10;
    case EVertexAttribute::Tex0:
    case EVertexAttribute::Tex1:
    case EVertexAttribute::Tex2:
    case EVertexAttribute::Tex3:
    case EVertexAttribute::Tex4:
    case EVertexAttribute::Tex5:
    case EVertexAttribute::Tex6:
    case EVertexAttribute::Tex7:
        return 0x08;
    case EVertexAttribute::BoneIndices:
        return 0x04;
    default:
        errorf("AttributeSize(): Unknown vertex attribute: %d", Attrib);
        return 0x00;
    }
}
