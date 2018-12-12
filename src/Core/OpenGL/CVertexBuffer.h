#ifndef CVERTEXBUFFER_H
#define CVERTEXBUFFER_H

#include "Core/Resource/TResPtr.h"
#include "Core/Resource/Animation/CSkin.h"
#include "Core/Resource/Model/CVertex.h"
#include "Core/Resource/Model/EVertexAttribute.h"
#include <vector>
#include <GL/glew.h>

class CVertexBuffer
{
    FVertexDescription mVtxDesc;            // Flags that indicate what vertex attributes are enabled on this vertex buffer
    GLuint mAttribBuffers[14];              // Separate GL buffer for each attribute to allow not tracking unused attribs. No support for matrix indices currently.
    TResPtr<CSkin> mpSkin;                  // Skin for skinned models. Null on unskinned models;
    std::vector<CVector3f> mPositions;      // Vector of vertex positions
    std::vector<CVector3f> mNormals;        // Vector of vertex normals
    std::vector<CColor> mColors[2];         // Vectors of vertex colors
    std::vector<CVector2f> mTexCoords[8];   // Vectors of texture coordinates
    std::vector<TBoneIndices> mBoneIndices; // Vectors of bone indices
    std::vector<TBoneWeights> mBoneWeights; // Vectors of bone weights
    bool mBuffered;                         // Bool value that indicates whether the attributes have been buffered.

public:
    CVertexBuffer();
    CVertexBuffer(FVertexDescription Desc);
    ~CVertexBuffer();
    uint16 AddVertex(const CVertex& rkVtx);
    uint16 AddIfUnique(const CVertex& rkVtx, uint16 Start);
    void Reserve(uint16 Size);
    void Clear();
    void Buffer();
    void Bind();
    void Unbind();
    bool IsBuffered();
    FVertexDescription VertexDesc();
    void SetVertexDesc(FVertexDescription Desc);
    void SetSkin(CSkin *pSkin);
    uint32 Size();
    GLuint CreateVAO();
};

#endif // CVERTEXBUFFER_H
