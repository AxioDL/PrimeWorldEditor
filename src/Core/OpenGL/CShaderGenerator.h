#ifndef SHADERGEN_H
#define SHADERGEN_H

#include "CShader.h"
#include "Core/Resource/CMaterial.h"
#include <GL/glew.h>

class CShaderGenerator
{
    CShader *mShader;

    CShaderGenerator();
    ~CShaderGenerator();
    bool CreateVertexShader(const CMaterial& Mat);
    bool CreatePixelShader(const CMaterial& Mat);

public:
    static CShader* GenerateShader(const CMaterial& Mat);
};

#endif // SHADERGEN_H
