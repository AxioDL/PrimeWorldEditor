#include "CShader.h"
#include "Core/Render/CGraphics.h"
#include <Common/BasicTypes.h>
#include <Common/Log.h>
#include <Common/TString.h>

#include <fstream>
#include <sstream>

bool gDebugDumpShaders = false;
uint64 gFailedCompileCount = 0;
uint64 gSuccessfulCompileCount = 0;

CShader* CShader::spCurrentShader = nullptr;
int CShader::smNumShaders = 0;

CShader::CShader()
{
    mVertexShaderExists = false;
    mPixelShaderExists = false;
    mProgramExists = false;
    smNumShaders++;
}

CShader::CShader(const char *pkVertexSource, const char *pkPixelSource)
{
    mVertexShaderExists = false;
    mPixelShaderExists = false;
    mProgramExists = false;
    smNumShaders++;

    CompileVertexSource(pkVertexSource);
    CompilePixelSource(pkPixelSource);
    LinkShaders();
}

CShader::~CShader()
{
    if (mVertexShaderExists) glDeleteShader(mVertexShader);
    if (mPixelShaderExists)  glDeleteShader(mPixelShader);
    if (mProgramExists)      glDeleteProgram(mProgram);

    if (spCurrentShader == this) spCurrentShader = 0;
    smNumShaders--;
}

bool CShader::CompileVertexSource(const char* pkSource)
{
    mVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(mVertexShader, 1, (const GLchar**) &pkSource, NULL);
    glCompileShader(mVertexShader);

    // Shader should be compiled - check for errors
    GLint CompileStatus;
    glGetShaderiv(mVertexShader, GL_COMPILE_STATUS, &CompileStatus);

    if (CompileStatus == GL_FALSE)
    {
        TString Out = "dump/BadVS_" + std::to_string(gFailedCompileCount) + ".txt";
        DumpShaderSource(mVertexShader, Out);
        errorf("Unable to compile vertex shader; dumped to %s", *Out);

        gFailedCompileCount++;
        glDeleteShader(mVertexShader);
        return false;
    }

    // Debug dump
    else if (gDebugDumpShaders == true)
    {
        TString Out = "dump/VS_" + TString::FromInt64(gSuccessfulCompileCount, 8, 10) + ".txt";
        DumpShaderSource(mVertexShader, Out);
        debugf("Debug shader dumping enabled; dumped to %s", *Out);

        gSuccessfulCompileCount++;
    }

    mVertexShaderExists = true;
    return true;
}

bool CShader::CompilePixelSource(const char* pkSource)
{
    mPixelShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(mPixelShader, 1, (const GLchar**) &pkSource, NULL);
    glCompileShader(mPixelShader);

    // Shader should be compiled - check for errors
    GLint CompileStatus;
    glGetShaderiv(mPixelShader, GL_COMPILE_STATUS, &CompileStatus);

    if (CompileStatus == GL_FALSE)
    {
        TString Out = "dump/BadPS_" + TString::FromInt64(gFailedCompileCount, 8, 10) + ".txt";
        errorf("Unable to compile pixel shader; dumped to %s", *Out);
        DumpShaderSource(mPixelShader, Out);

        gFailedCompileCount++;
        glDeleteShader(mPixelShader);
        return false;
    }

    // Debug dump
    else if (gDebugDumpShaders == true)
    {
        TString Out = "dump/PS_" + TString::FromInt64(gSuccessfulCompileCount, 8, 10) + ".txt";
        debugf("Debug shader dumping enabled; dumped to %s", *Out);
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
        TString Out = "dump/BadLink_" + TString::FromInt64(gFailedCompileCount, 8, 10) + ".txt";
        errorf("Unable to link shaders. Dumped error log to %s", *Out);

        GLint LogLen;
        glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &LogLen);
        GLchar *pInfoLog = new GLchar[LogLen];
        glGetProgramInfoLog(mProgram, LogLen, NULL, pInfoLog);

        std::ofstream LinkOut;
        LinkOut.open(*Out);

        if (LogLen > 0)
            LinkOut << pInfoLog;

        LinkOut.close();
        delete[] pInfoLog;

        gFailedCompileCount++;
        glDeleteProgram(mProgram);
        return false;
    }

    mMVPBlockIndex = GetUniformBlockIndex("MVPBlock");
    mVertexBlockIndex = GetUniformBlockIndex("VertexBlock");
    mPixelBlockIndex = GetUniformBlockIndex("PixelBlock");
    mLightBlockIndex = GetUniformBlockIndex("LightBlock");
    mBoneTransformBlockIndex = GetUniformBlockIndex("BoneTransformBlock");

    CacheCommonUniforms();
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

GLuint CShader::GetUniformLocation(const char* pkUniform)
{
    return glGetUniformLocation(mProgram, pkUniform);
}

GLuint CShader::GetUniformBlockIndex(const char* pkUniformBlock)
{
    return glGetUniformBlockIndex(mProgram, pkUniformBlock);
}

void CShader::SetTextureUniforms(uint32 NumTextures)
{
    for (uint32 iTex = 0; iTex < NumTextures; iTex++)
        glUniform1i(mTextureUniforms[iTex], iTex);
}

void CShader::SetNumLights(uint32 NumLights)
{
    glUniform1i(mNumLightsUniform, NumLights);
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
        glUniformBlockBinding(mProgram, mBoneTransformBlockIndex, CGraphics::BoneTransformBlockBindingPoint());
    }
}

// ************ STATIC ************
CShader* CShader::FromResourceFile(const TString& rkShaderName)
{
    TString VertexShaderFilename = "../resources/shaders/" + rkShaderName + ".vs";
    TString PixelShaderFilename = "../resources/shaders/" + rkShaderName + ".ps";
    TString VertexShaderText, PixelShaderText;

    if (!FileUtil::LoadFileToString(VertexShaderFilename, VertexShaderText))
        errorf("Couldn't load vertex shader file for %s", *rkShaderName);
    if (!FileUtil::LoadFileToString(PixelShaderFilename, PixelShaderText))
        errorf("Couldn't load pixel shader file for %s", *rkShaderName);
    if (VertexShaderText.IsEmpty() || PixelShaderText.IsEmpty())
        return nullptr;

    CShader *pShader = new CShader();
    pShader->CompileVertexSource(*VertexShaderText);
    pShader->CompilePixelSource(*PixelShaderText);
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
void CShader::CacheCommonUniforms()
{
    for (uint32 iTex = 0; iTex < 8; iTex++)
    {
        TString TexUniform = "Texture" + TString::FromInt32(iTex);
        mTextureUniforms[iTex] = glGetUniformLocation(mProgram, *TexUniform);
    }

    mNumLightsUniform = glGetUniformLocation(mProgram, "NumLights");
}

void CShader::DumpShaderSource(GLuint Shader, const TString& rkOut)
{
    GLint SourceLen;
    glGetShaderiv(Shader, GL_SHADER_SOURCE_LENGTH, &SourceLen);
    GLchar *Source = new GLchar[SourceLen];
    glGetShaderSource(Shader, SourceLen, NULL, Source);

    GLint LogLen;
    glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &LogLen);
    GLchar *pInfoLog = new GLchar[LogLen];
    glGetShaderInfoLog(Shader, LogLen, NULL, pInfoLog);

    std::ofstream ShaderOut;
    ShaderOut.open(*rkOut);

    if (SourceLen > 0)
        ShaderOut << Source;
    if (LogLen > 0)
        ShaderOut << pInfoLog;

    ShaderOut.close();

    delete[] Source;
    delete[] pInfoLog;
}
