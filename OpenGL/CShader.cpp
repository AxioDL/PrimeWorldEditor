#include "CShader.h"
#include <Common/types.h>
#include <Core/CGraphics.h>
#include <FileIO/CTextInStream.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

bool gDebugDumpShaders = false;
u64 gFailedCompileCount = 0;
u64 gSuccessfulCompileCount = 0;

CShader* CShader::spCurrentShader = nullptr;

CShader::CShader()
{
    mVertexShaderExists = false;
    mPixelShaderExists = false;
    mProgramExists = false;
}

CShader::CShader(const char *kpVertexSource, const char *kpPixelSource)
{
    mVertexShaderExists = false;
    mPixelShaderExists = false;
    mProgramExists = false;

    CompileVertexSource(kpVertexSource);
    CompilePixelSource(kpPixelSource);
    LinkShaders();
}

CShader::~CShader()
{
    if (mVertexShaderExists) glDeleteShader(mVertexShader);
    if (mPixelShaderExists)  glDeleteShader(mPixelShader);
    if (mProgramExists)      glDeleteProgram(mProgram);

    if (spCurrentShader == this) spCurrentShader = 0;
}

bool CShader::CompileVertexSource(const char* kpSource)
{
    mVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(mVertexShader, 1, (const GLchar**) &kpSource, NULL);
    glCompileShader(mVertexShader);

    // Shader should be compiled - check for errors
    GLint CompileStatus;
    glGetShaderiv(mVertexShader, GL_COMPILE_STATUS, &CompileStatus);

    if (CompileStatus == GL_FALSE)
    {
        std::string Out = "dump/BadVS_" + std::to_string(gFailedCompileCount) + ".txt";
        std::cout << "ERROR: Unable to compile vertex shader; dumped to " << Out << "\n";
        DumpShaderSource(mVertexShader, Out);

        gFailedCompileCount++;
        glDeleteShader(mVertexShader);
        return false;
    }

    // Debug dump
    else if (gDebugDumpShaders == true)
    {
        std::string Out = "dump/VS_" + std::to_string(gSuccessfulCompileCount) + ".txt";
        std::cout << "Debug shader dumping enabled; dumped to " << Out << "\n";
        DumpShaderSource(mVertexShader, Out);

        gSuccessfulCompileCount++;
    }

    mVertexShaderExists = true;
    return true;
}

bool CShader::CompilePixelSource(const char* kpSource)
{
    mPixelShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(mPixelShader, 1, (const GLchar**) &kpSource, NULL);
    glCompileShader(mPixelShader);

    // Shader should be compiled - check for errors
    GLint CompileStatus;
    glGetShaderiv(mPixelShader, GL_COMPILE_STATUS, &CompileStatus);

    if (CompileStatus == GL_FALSE)
    {
        std::string Out = "dump/BadPS_" + std::to_string(gFailedCompileCount) + ".txt";
        std::cout << "ERROR: Unable to compile pixel shader; dumped to " << Out << "\n";
        DumpShaderSource(mPixelShader, Out);

        gFailedCompileCount++;
        glDeleteShader(mPixelShader);
        return false;
    }

    // Debug dump
    else if (gDebugDumpShaders == true)
    {
        std::string Out = "dump/PS_" + std::to_string(gSuccessfulCompileCount) + ".txt";
        std::cout << "Debug shader dumping enabled; dumped to " << Out << "\n";
        DumpShaderSource(mPixelShader, Out);

        gSuccessfulCompileCount++;
    }

    mPixelShaderExists = true;
    return true;
}

bool CShader::LinkShaders()
{
    if ((!mVertexShaderExists) || (!mPixelShaderExists)) return false;

    mProgram = glCreateProgram();
    glAttachShader(mProgram, mVertexShader);
    glAttachShader(mProgram, mPixelShader);
    glLinkProgram(mProgram);

    glDeleteShader(mVertexShader);
    glDeleteShader(mPixelShader);
    mVertexShaderExists = false;
    mPixelShaderExists = false;

    // Shader should be linked - check for errors
    GLint LinkStatus;
    glGetProgramiv(mProgram, GL_LINK_STATUS, &LinkStatus);

    if (LinkStatus == GL_FALSE)
    {
        std::string Out = "dump/BadLink_" + std::to_string(gFailedCompileCount) + ".txt";
        std::cout << "ERROR: Unable to link shaders. Dumped error log to " << Out << "\n";

        GLint LogLen;
        glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &LogLen);
        GLchar *InfoLog = new GLchar[LogLen];
        glGetProgramInfoLog(mProgram, LogLen, NULL, InfoLog);

        std::ofstream LinkOut;
        LinkOut.open(Out.c_str());

        if (LogLen > 0)
            LinkOut << InfoLog;

        LinkOut.close();
        delete[] InfoLog;

        gFailedCompileCount++;
        glDeleteProgram(mProgram);
        return false;
    }

    mMVPBlockIndex = GetUniformBlockIndex("MVPBlock");
    mVertexBlockIndex = GetUniformBlockIndex("VertexBlock");
    mPixelBlockIndex = GetUniformBlockIndex("PixelBlock");
    mLightBlockIndex = GetUniformBlockIndex("LightBlock");

    mProgramExists = true;
    return true;
}

bool CShader::IsValidProgram()
{
    return mProgramExists;
}

GLuint CShader::GetProgramID()
{
    return mProgram;
}

GLuint CShader::GetUniformLocation(const char* Uniform)
{
    return glGetUniformLocation(mProgram, Uniform);
}

GLuint CShader::GetUniformBlockIndex(const char* UniformBlock)
{
    return glGetUniformBlockIndex(mProgram, UniformBlock);
}

void CShader::SetCurrent()
{
    if (spCurrentShader != this)
    {
        glUseProgram(mProgram);
        spCurrentShader = this;

        glUniformBlockBinding(mProgram, mMVPBlockIndex, CGraphics::MVPBlockBindingPoint());
        glUniformBlockBinding(mProgram, mVertexBlockIndex, CGraphics::VertexBlockBindingPoint());
        glUniformBlockBinding(mProgram, mPixelBlockIndex, CGraphics::PixelBlockBindingPoint());
        glUniformBlockBinding(mProgram, mLightBlockIndex, CGraphics::LightBlockBindingPoint());
    }
}

// ************ STATIC ************
CShader* CShader::FromResourceFile(std::string ShaderName)
{
    std::string VertexShaderFilename = "../resources/shaders/" + ShaderName + ".vs";
    std::string PixelShaderFilename = "../resources/shaders/" + ShaderName + ".ps";
    CTextInStream VertexShaderFile(VertexShaderFilename);
    CTextInStream PixelShaderFile(PixelShaderFilename);

    if (!VertexShaderFile.IsValid())
        std::cout << "Error: Couldn't load vertex shader file for " << ShaderName << "\n";
    if (!PixelShaderFile.IsValid())
        std::cout << "Error: Couldn't load pixel shader file for " << ShaderName << "\n";
    if ((!VertexShaderFile.IsValid()) || (!PixelShaderFile.IsValid())) return nullptr;

    std::stringstream VertexShader;
    while (!VertexShaderFile.EoF())
        VertexShader << VertexShaderFile.GetString();

    std::stringstream PixelShader;
    while (!PixelShaderFile.EoF())
        PixelShader << PixelShaderFile.GetString();

    CShader *pShader = new CShader();
    pShader->CompileVertexSource(VertexShader.str().c_str());
    pShader->CompilePixelSource(PixelShader.str().c_str());
    pShader->LinkShaders();
    return pShader;
}

CShader* CShader::CurrentShader()
{
    return spCurrentShader;
}

void CShader::KillCachedShader()
{
    spCurrentShader = 0;
}

// ************ PRIVATE ************
void CShader::DumpShaderSource(GLuint Shader, std::string Out)
{
    GLint SourceLen;
    glGetShaderiv(Shader, GL_SHADER_SOURCE_LENGTH, &SourceLen);
    GLchar *Source = new GLchar[SourceLen];
    glGetShaderSource(Shader, SourceLen, NULL, Source);

    GLint LogLen;
    glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &LogLen);
    GLchar *InfoLog = new GLchar[LogLen];
    glGetShaderInfoLog(Shader, LogLen, NULL, InfoLog);

    std::ofstream ShaderOut;
    ShaderOut.open(Out.c_str());

    if (SourceLen > 0)
        ShaderOut << Source;
    if (LogLen > 0)
        ShaderOut << InfoLog;

    ShaderOut.close();

    delete[] Source;
    delete[] InfoLog;
}
