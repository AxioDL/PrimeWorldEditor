#ifndef CFRAMEBUFFER_H
#define CFRAMEBUFFER_H

#include "CRenderbuffer.h"
#include "Core/Resource/Texture/CTexture.h"
#include <GL/glew.h>

class CFramebuffer
{
    GLuint mFramebuffer;
    CRenderbuffer *mpRenderbuffer;
    CTexture *mpTexture;
    uint32 mWidth, mHeight;
    bool mEnableMultisampling;
    bool mInitialized;
    GLenum mStatus;

    static GLint smDefaultFramebuffer;
    static bool smStaticsInitialized;

public:
    CFramebuffer();
    CFramebuffer(uint32 Width, uint32 Height);
    ~CFramebuffer();
    void Init();
    void Bind(GLenum Target = GL_FRAMEBUFFER);
    void Resize(uint32 Width, uint32 Height);
    void SetMultisamplingEnabled(bool Enable);

    // Accessors
    inline CTexture* Texture() const    { return mpTexture; }

    // Static
    static void BindDefaultFramebuffer(GLenum Target = GL_FRAMEBUFFER);

protected:
    void InitBuffers();
};

#endif // CFRAMEBUFFER_H
