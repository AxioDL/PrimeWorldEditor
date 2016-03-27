#ifndef CTEXTURE_H
#define CTEXTURE_H

#include "CResource.h"
#include "ETexelFormat.h"
#include <FileIO/FileIO.h>
#include <Common/types.h>
#include <Math/CVector2f.h>

#include <GL/glew.h>

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

    bool mBufferExists;  // Indicates whether image data buffer has valid data
    u8 *mpImgDataBuffer; // Pointer to image data buffer
    u32 mImgDataSize;    // Size of image data buffer

    bool mGLBufferExists; // Indicates whether GL buffer has valid data
    GLuint mTextureID;    // ID for texture GL buffer

public:
    CTexture();
    CTexture(u32 Width, u32 Height);
    ~CTexture();

    bool BufferGL();
    void Bind(u32 GLTextureUnit);
    void Resize(u32 Width, u32 Height);
    float ReadTexelAlpha(const CVector2f& rkTexCoord);
    bool WriteDDS(IOutputStream& rOut);

    // Getters
    ETexelFormat TexelFormat() const        { return mTexelFormat; }
    ETexelFormat SourceTexelFormat() const  { return mSourceTexelFormat; }
    u32 Width() const           { return (u32) mWidth; }
    u32 Height() const          { return (u32) mHeight; }
    u32 NumMipMaps() const      { return mNumMipMaps; }
    GLuint TextureID() const    { return mTextureID; }

    // Static
    static u32 FormatBPP(ETexelFormat Format);

    // Private
private:
    void CalcLinearSize();
    u32 CalcTotalSize();
    void CopyGLBuffer();
    void DeleteBuffers();
};

#endif // CTEXTURE_H
