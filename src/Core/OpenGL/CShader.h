#ifndef CSHADER_H
#define CSHADER_H

#include <gl/glew.h>
#include <Common/TString.h>

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

    static CShader* spCurrentShader;

public:
    CShader();
    CShader(const char* kpVertexSource, const char* kpPixelSource);
    ~CShader();
    bool CompileVertexSource(const char* kpSource);
    bool CompilePixelSource(const char* kpSource);
    bool LinkShaders();
    bool IsValidProgram();
    GLuint GetProgramID();
    GLuint GetUniformLocation(const char* kpUniform);
    GLuint GetUniformBlockIndex(const char* kpUniformBlock);
    void SetCurrent();

    // Static
    static CShader* FromResourceFile(const TString& ShaderName);
    static CShader* CurrentShader();
    static void KillCachedShader();

private:
    void DumpShaderSource(GLuint Shader, const TString& Out);
};

#endif // CSHADER_H
