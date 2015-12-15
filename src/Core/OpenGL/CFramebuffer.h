#ifndef CFRAMEBUFFER_H
#define CFRAMEBUFFER_H

#include "CRenderbuffer.h"
#include "Core/Resource/CTexture.h"
#include <GL/glew.h>

class CFramebuffer
{
    GLuint mFramebuffer;
    CRenderbuffer *mpRenderbuffer;
    CTexture *mpTexture;
    u32 mWidth, mHeight;
    bool mInitialized;
    GLenum mStatus;

    static GLint smDefaultFramebuffer;
    static bool smStaticsInitialized;

public:
    CFramebuffer();
    CFramebuffer(u32 Width, u32 Height);
    ~CFramebuffer();
    void Init();
    void Bind();
    void Resize(u32 Width, u32 Height);
    CTexture* Texture();
    static void BindDefaultFramebuffer();

};

#endif // CFRAMEBUFFER_H
