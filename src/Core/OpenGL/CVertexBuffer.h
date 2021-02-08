#ifndef CVERTEXBUFFER_H
#define CVERTEXBUFFER_H

#include "Core/Resource/TResPtr.h"
#include "Core/Resource/Animation/CSkin.h"
#include "Core/Resource/Model/CVertex.h"
#include "Core/Resource/Model/EVertexAttribute.h"
#include <array>
#include <vector>
#include <GL/glew.h>

class CVertexBuffer
{
    FVertexDescription mVtxDesc;                      // Flags that indicate what vertex attributes are enabled on this vertex buffer
    std::array<GLuint, 14> mAttribBuffers{};          // Separate GL buffer for each attribute to allow not tracking unused attribs. No support for matrix indices currently.
    TResPtr<CSkin> mpSkin;                            // Skin for skinned models. Null on unskinned models;
    std::vector<CVector3f> mPositions;                // Vector of vertex positions
    std::vector<CVector3f> mNormals;                  // Vector of vertex normals
    std::array<std::vector<CColor>, 2> mColors;       // Vectors of vertex colors
    std::array<std::vector<CVector2f>, 8> mTexCoords; // Vectors of texture coordinates
    std::vector<TBoneIndices> mBoneIndices;           // Vectors of bone indices
    std::vector<TBoneWeights> mBoneWeights;           // Vectors of bone weights
    bool mBuffered = false;                           // Bool value that indicates whether the attributes have been buffered.

public:
    CVertexBuffer();
    explicit CVertexBuffer(FVertexDescription Desc);
    ~CVertexBuffer();
    uint16 AddVertex(const CVertex& rkVtx);
    uint16 AddIfUnique(const CVertex& rkVtx, uint16 Start);
    void Reserve(size_t Size);
    void Clear();
    void Buffer();
    void Bind();
    void Unbind();
    bool IsBuffered() const;
    FVertexDescription VertexDesc() const;
    void SetVertexDesc(FVertexDescription Desc);
    void SetSkin(CSkin *pSkin);
    size_t Size() const;
    GLuint CreateVAO();
};

#endif // CVERTEXBUFFER_H
