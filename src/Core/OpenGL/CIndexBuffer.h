#ifndef CINDEXBUFFER_H
#define CINDEXBUFFER_H

#include <Common/types.h>
#include <Math/CVector3f.h>
#include <GL/glew.h>

class CIndexBuffer
{
    GLuint mIndexBuffer;
    std::vector<u16> mIndices;
    GLenum mPrimitiveType;
    bool mBuffered;

public:
    CIndexBuffer();
    CIndexBuffer(GLenum type);
    ~CIndexBuffer();
    void AddIndex(u16 idx);
    void AddIndices(u16 *indicesPtr, u32 count);
    void Reserve(u32 size);
    void Clear();
    void Buffer();
    void Bind();
    void Unbind();
    void DrawElements();
    void DrawElements(u32 Offset, u32 Size);
    bool IsBuffered();

    u32 GetSize();
    GLenum GetPrimitiveType();
    void SetPrimitiveType(GLenum type);

    void TrianglesToStrips(u16 *indicesPtr, u32 count);
    void FansToStrips(u16 *indicesPtr, u32 count);
    void QuadsToStrips(u16 *indicesPtr, u32 count);
};

#endif // CINDEXBUFFER_H
