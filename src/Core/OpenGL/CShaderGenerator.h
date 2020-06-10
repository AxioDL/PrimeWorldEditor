#ifndef SHADERGEN_H
#define SHADERGEN_H

#include "CShader.h"
#include "Core/Resource/CMaterial.h"
#include <GL/glew.h>

/**
 * @todo Would be great to have a more complex shader system that would allow
 * more advanced things to be done with shaders and materials in particular.
 * Currently every material only has one shader, which means any extra rendering
 * effects that need to be rendered on a mesh that has a material has to be
 * integrated directly into that material's shader and has to be toggled via a
 * shader uniform, which is pretty messy. If you look at Unreal for instance,
 * there is a much nicer system where the output of a material (color, emissive,
 * opacity, etc) is provided via a function call in the shader code, and then
 * you can write another shader on top that calls that function and then integrates
 * the material output into the shader's output pixel color however you want, which
 * allows for vastly more customization in how materials render for any given situation.
 * As the current system stands, it's kind of a pain to extend with any new features,
 * or to add any new graphical effects to game assets.
 */
class CShaderGenerator
{
    CShader *mpShader = nullptr;

    CShaderGenerator();
    ~CShaderGenerator();
    bool CreateVertexShader(const CMaterial& rkMat);
    bool CreatePixelShader(const CMaterial& rkMat);

public:
    static CShader* GenerateShader(const CMaterial& rkMat);
};

#endif // SHADERGEN_H
