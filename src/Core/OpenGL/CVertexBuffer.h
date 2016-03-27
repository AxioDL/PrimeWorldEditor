#ifndef CVERTEXBUFFER_H
#define CVERTEXBUFFER_H

#include "Core/Resource/Model/CVertex.h"
#include "Core/Resource/Model/EVertexAttribute.h"
#include <vector>
#include <GL/glew.h>

class CVertexBuffer
{
    FVertexDescription mVtxDesc;          // Flags that indicate what vertex attributes are enabled on this vertex buffer
    GLuint mAttribBuffers[12];            // Separate GL buffer for each attribute to allow not tracking unused attribs. No support for matrix indices currently.
    std::vector<CVector3f> mPositions;    // Vector of vertex positions
    std::vector<CVector3f> mNormals;      // Vector of vertex normals
    std::vector<CColor> mColors[2];       // Vectors of vertex colors
    std::vector<CVector2f> mTexCoords[8]; // Vectors of texture coordinates
    bool mBuffered;                       // Bool value that indicates whether the attributes have been buffered.

public:
    CVertexBuffer();
    CVertexBuffer(FVertexDescription Desc);
    ~CVertexBuffer();
    u16 AddVertex(const CVertex& rkVtx);
    u16 AddIfUnique(const CVertex& rkVtx, u16 Start);
    void Reserve(u16 Size);
    void Clear();
    void Buffer();
    void Bind();
    void Unbind();
    bool IsBuffered();
    FVertexDescription VertexDesc();
    void SetVertexDesc(FVertexDescription Desc);
    u32 Size();
    GLuint CreateVAO();
};

#endif // CVERTEXBUFFER_H
