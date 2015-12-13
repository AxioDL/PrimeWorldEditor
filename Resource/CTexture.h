#ifndef CTEXTURE_H
#define CTEXTURE_H

#include <Common/types.h>
#include <Common/CVector2f.h>
#include <FileIO/FileIO.h>
#include <gl/glew.h>
#include "CResource.h"
#include "ETexelFormat.h"

class CTexture : public CResource
{
    DECLARE_RESOURCE_TYPE(eTexture)
    friend class CTextureDecoder;
    friend class CTextureEncoder;

    ETexelFormat mTexelFormat;         // Format of decoded image data
    ETexelFormat mSourceTexelFormat;   // Format of input TXTR file
    u16 mWidth, mHeight;               // Image dimensions
    u32 mNumMipMaps;                   // The number of mipmaps this texture has
    u32 mLinearSize;                   // The size of the top level mipmap, in bytes

    bool mBufferExists; // Boolean that indicates whether image data buffer has valid data
    u8 *mImgDataBuffer; // Pointer to image data buffer
    u32 mImgDataSize;   // Size of image data buffer

    bool mGLBufferExists; // Boolean that indicates whether GL buffer has valid data
    GLuint mTextureID;    // ID for texture GL buffer

public:
    CTexture();
    CTexture(const CTexture& Source);
    CTexture(u32 Width, u32 Height);
    ~CTexture();

    bool BufferGL();
    void Bind(u32 GLTextureUnit);
    void Resize(u32 Width, u32 Height);
    float ReadTexelAlpha(const CVector2f& TexCoord);
    bool WriteDDS(COutputStream& out);

    // Getters
    ETexelFormat TexelFormat();
    ETexelFormat SourceTexelFormat();
    u32 Width();
    u32 Height();
    u32 NumMipMaps();
    GLuint TextureID();

    // Static
    static u32 FormatBPP(ETexelFormat Format);

    // Private
private:
    void CalcLinearSize();
    u32 CalcTotalSize();
    void CopyGLBuffer();
    void DeleteBuffers();
};

inline ETexelFormat CTexture::TexelFormat() {
    return mTexelFormat;
}

inline ETexelFormat CTexture::SourceTexelFormat() {
    return mSourceTexelFormat;
}

inline u32 CTexture::Width() {
    return (u32) mWidth;
}

inline u32 CTexture::Height() {
    return (u32) mHeight;
}

inline u32 CTexture::NumMipMaps() {
    return mNumMipMaps;
}

inline GLuint CTexture::TextureID() {
    return mTextureID;
}

#endif // CTEXTURE_H
