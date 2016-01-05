#ifndef CDYNAMICVERTEXBUFFER_H
#define CDYNAMICVERTEXBUFFER_H

#include "Core/Resource/Model/EVertexAttribute.h"
#include <Common/types.h>
#include <Math/CVector2f.h>
#include <Math/CVector3f.h>

#include <vector>
#include <GL/glew.h>

class CDynamicVertexBuffer
{
    FVertexDescription mAttribFlags;
    FVertexDescription mBufferedFlags;
    u32 mNumVertices;
    GLuint mAttribBuffers[12];

public:
    CDynamicVertexBuffer();
    ~CDynamicVertexBuffer();
    void SetVertexCount(u32 NumVerts);
    void Bind();
    void Unbind();
    void SetActiveAttribs(FVertexDescription AttribFlags);
    void BufferAttrib(EVertexAttribute Attrib, const void *pData);
    void ClearBuffers();
    GLuint CreateVAO();
private:
    void InitBuffers();
};

#endif // CDYNAMICVERTEXBUFFER_H
