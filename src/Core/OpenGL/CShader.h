#ifndef CSHADER_H
#define CSHADER_H

#include <Common/TString.h>
#include <GL/glew.h>

class CShader
{
    bool mVertexShaderExists;
    bool mPixelShaderExists;
    bool mProgramExists;
    GLuint mVertexShader;
    GLuint mPixelShader;
    GLuint mProgram;

    GLuint mMVPBlockIndex;
    GLuint mVertexBlockIndex;
    GLuint mPixelBlockIndex;
    GLuint mLightBlockIndex;
    GLuint mBoneTransformBlockIndex;

    // Cached uniform locations
    GLint mTextureUniforms[8];
    GLint mNumLightsUniform;

    static int smNumShaders;
    static CShader* spCurrentShader;

public:
    CShader();
    CShader(const char* pkVertexSource, const char* pkPixelSource);
    ~CShader();
    bool CompileVertexSource(const char* pkSource);
    bool CompilePixelSource(const char* pkSource);
    bool LinkShaders();
    bool IsValidProgram();
    GLuint GetProgramID();
    GLuint GetUniformLocation(const char* pkUniform);
    GLuint GetUniformBlockIndex(const char* pkUniformBlock);
    void SetTextureUniforms(uint32 NumTextures);
    void SetNumLights(uint32 NumLights);
    void SetCurrent();

    // Static
    static CShader* FromResourceFile(const TString& rkShaderName);
    static CShader* CurrentShader();
    static void KillCachedShader();

    inline static int NumShaders() { return smNumShaders; }

private:
    void CacheCommonUniforms();
    void DumpShaderSource(GLuint Shader, const TString& rkOut);
};

#endif // CSHADER_H
