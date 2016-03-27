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
    CIndexBuffer(GLenum Type);
    ~CIndexBuffer();
    void AddIndex(u16 Index);
    void AddIndices(u16 *pIndices, u32 Count);
    void Reserve(u32 Size);
    void Clear();
    void Buffer();
    void Bind();
    void Unbind();
    void DrawElements();
    void DrawElements(u32 Offset, u32 Size);
    bool IsBuffered();

    u32 GetSize();
    GLenum GetPrimitiveType();
    void SetPrimitiveType(GLenum Type);

    void TrianglesToStrips(u16 *pIndices, u32 Count);
    void FansToStrips(u16 *pIndices, u32 Count);
    void QuadsToStrips(u16 *pIndices, u32 Count);
};

#endif // CINDEXBUFFER_H
