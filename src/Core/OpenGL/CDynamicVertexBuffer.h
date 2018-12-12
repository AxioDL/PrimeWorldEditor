#ifndef CDYNAMICVERTEXBUFFER_H
#define CDYNAMICVERTEXBUFFER_H

#include "Core/Resource/Model/EVertexAttribute.h"
#include <Common/BasicTypes.h>

#include <vector>
#include <GL/glew.h>

class CDynamicVertexBuffer
{
    FVertexDescription mAttribFlags;
    FVertexDescription mBufferedFlags;
    uint32 mNumVertices;
    GLuint mAttribBuffers[12];

public:
    CDynamicVertexBuffer();
    ~CDynamicVertexBuffer();
    void SetVertexCount(uint32 NumVerts);
    void Bind();
    void Unbind();
    void SetActiveAttribs(FVertexDescription AttribFlags);
    void BufferAttrib(EVertexAttribute Attrib, const void *pkData);
    void ClearBuffers();
    GLuint CreateVAO();
private:
    void InitBuffers();
};

#endif // CDYNAMICVERTEXBUFFER_H
