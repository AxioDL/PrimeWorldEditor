#ifndef SHADERGEN_H
#define SHADERGEN_H

#include "CShader.h"
#include "Core/Resource/CMaterial.h"
#include <GL/glew.h>

class CShaderGenerator
{
    CShader *mpShader;

    CShaderGenerator();
    ~CShaderGenerator();
    bool CreateVertexShader(const CMaterial& rkMat);
    bool CreatePixelShader(const CMaterial& rkMat);

public:
    static CShader* GenerateShader(const CMaterial& rkMat);
};

#endif // SHADERGEN_H
