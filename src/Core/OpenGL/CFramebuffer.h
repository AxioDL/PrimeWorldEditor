#ifndef CFRAMEBUFFER_H
#define CFRAMEBUFFER_H

#include "CRenderbuffer.h"
#include "Core/Resource/CTexture.h"
#include <GL/glew.h>

class CFramebuffer
{
    GLuint mFramebuffer = 0;
    CRenderbuffer *mpRenderbuffer = nullptr;
    CTexture *mpTexture = nullptr;
    uint32 mWidth = 0;
    uint32 mHeight = 0;
    bool mEnableMultisampling = false;
    bool mInitialized = false;
    GLenum mStatus{};

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
    CTexture* Texture() const    { return mpTexture; }

    // Static
    static void BindDefaultFramebuffer(GLenum Target = GL_FRAMEBUFFER);

protected:
    void InitBuffers();
};

#endif // CFRAMEBUFFER_H
