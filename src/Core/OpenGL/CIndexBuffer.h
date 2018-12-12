#ifndef CINDEXBUFFER_H
#define CINDEXBUFFER_H

#include <Common/BasicTypes.h>
#include <Common/Math/CVector3f.h>
#include <GL/glew.h>

class CIndexBuffer
{
    GLuint mIndexBuffer;
    std::vector<uint16> mIndices;
    GLenum mPrimitiveType;
    bool mBuffered;

public:
    CIndexBuffer();
    CIndexBuffer(GLenum Type);
    ~CIndexBuffer();
    void AddIndex(uint16 Index);
    void AddIndices(uint16 *pIndices, uint Count);
    void Reserve(uint Size);
    void Clear();
    void Buffer();
    void Bind();
    void Unbind();
    void DrawElements();
    void DrawElements(uint Offset, uint Size);
    bool IsBuffered();

    uint GetSize();
    GLenum GetPrimitiveType();
    void SetPrimitiveType(GLenum Type);

    void TrianglesToStrips(uint16 *pIndices, uint Count);
    void FansToStrips(uint16 *pIndices, uint Count);
    void QuadsToStrips(uint16 *pIndices, uint Count);
};

#endif // CINDEXBUFFER_H
