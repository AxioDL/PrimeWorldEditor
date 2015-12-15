#ifndef CRENDERBUFFER_H
#define CRENDERBUFFER_H

#include <Common/types.h>
#include <GL/glew.h>

class CRenderbuffer
{
    GLuint mRenderbuffer;
    u32 mWidth, mHeight;
    bool mInitialized;

public:
    CRenderbuffer();
    CRenderbuffer(u32 Width, u32 Height);
    ~CRenderbuffer();
    void Init();
    void Resize(u32 Width, u32 Height);
    void Bind();
    void Unbind();

    // Getters
    GLuint BufferID();
};

inline GLuint CRenderbuffer::BufferID()
{
    return mRenderbuffer;
}

#endif // CRENDERBUFFER_H
