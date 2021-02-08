#ifndef CINDEXBUFFER_H
#define CINDEXBUFFER_H

#include <Common/BasicTypes.h>
#include <Common/Math/CVector3f.h>
#include <GL/glew.h>

class CIndexBuffer
{
    GLuint mIndexBuffer = 0;
    std::vector<uint16> mIndices;
    GLenum mPrimitiveType{};
    bool mBuffered = false;

public:
    CIndexBuffer();
    explicit CIndexBuffer(GLenum type);
    ~CIndexBuffer();
    void AddIndex(uint16 index);
    void AddIndices(const uint16 *indices, size_t count);
    void Reserve(size_t size);
    void Clear();
    void Buffer();
    void Bind();
    void Unbind();
    void DrawElements();
    void DrawElements(uint offset, uint size);
    bool IsBuffered() const;

    uint GetSize() const;
    GLenum GetPrimitiveType() const;
    void SetPrimitiveType(GLenum type);

    void TrianglesToStrips(uint16 *indices, size_t count);
    void FansToStrips(uint16 *indices, size_t count);
    void QuadsToStrips(uint16 *indices, size_t count);
};

#endif // CINDEXBUFFER_H
