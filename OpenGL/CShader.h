#ifndef CSHADER_H
#define CSHADER_H

#include <gl/glew.h>
#include <string>

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
    GLuint GetUniformLocation(const char* Uniform);
    GLuint GetUniformBlockIndex(const char* UniformBlock);
    void SetCurrent();

    // Static
    static CShader* FromResourceFile(std::string ShaderName);
    static CShader* CurrentShader();
    static void KillCachedShader();

private:
    void DumpShaderSource(GLuint Shader, std::string Out);
};

#endif // CSHADER_H
