#ifndef CTEXTURE_H
#define CTEXTURE_H

#include "Core/Resource/CResource.h"
#include "ETexelFormat.h"
#include <Common/BasicTypes.h>
#include <Common/FileIO.h>
#include <Common/Math/CVector2f.h>

#include <GL/glew.h>

/** Mipmap data */
struct SMipData
{
    /** Mip dimensions */
    uint SizeX, SizeY;

    /** Mip texel data */
    std::vector<uint8> DataBuffer;

    /** Mip texel data with game texel formats; may be empty when not in use */
    std::vector<uint8> GameDataBuffer;
};

/** Class representing a 2D texture asset */
class CTexture : public CResource
{
    DECLARE_RESOURCE_TYPE(Texture)
    friend class CTextureDecoder;
    friend class CTextureEncoder;

    /** Format of encoded image data in-game */
    EGXTexelFormat mGameFormat;

    /** Format of decoded image data in-editor */
    ETexelFormat mEditorFormat;

    /** Mipmap data */
    std::vector<SMipData> mMipData;

    /** @todo the following is OpenGL stuff that really shouldn't be implemented here */
    /** Whether multisample should be enabled (if this texture is a render target). */
    bool mEnableMultisampling;

    /** OpenGL texture resource handle */
    GLuint mTextureResource;

public:
    /** Constructors */
    CTexture(CResourceEntry* pEntry = 0);
    CTexture(uint SizeX, uint SizeY);
    ~CTexture();

    /** Generate mipmap chain based on the contents of the first mip */
    void GenerateMipTail(uint NumMips = 0);

    /** Allocate mipmap data, but does not fill any data. Returns the new mipmap count. */
    uint AllocateMipTail(uint DesiredMipCount = 0);

    /** Compress the texture data into the format specified by Format. */
    void Compress(EGXTexelFormat Format);

    /** Generate editor texel data based on the currently loaded game texel data */
    void GenerateEditorData(bool bClearGameData = true);

    /**
     * Update the internal resolution of the texture; used for dynamically-scaling textures
     * @todo - should not be implemented here as these textures are not relevant to texture assets
     */
    void Resize(uint SizeX, uint SizeY);

    /** Return the alpha value of the texel at the given coordinates */
    float ReadTexelAlpha(const CVector2f& rkTexCoord);

    /** Create/release resources for rendering this texture */
    void CreateRenderResources();
    void ReleaseRenderResources();

    // Accessors
    FORCEINLINE ETexelFormat EditorTexelFormat() const  { return mEditorFormat; }
    FORCEINLINE EGXTexelFormat GameTexelFormat() const  { return mGameFormat; }
    FORCEINLINE uint SizeX() const                      { return mMipData.empty() ? 0 : mMipData[0].SizeX; }
    FORCEINLINE uint SizeY() const                      { return mMipData.empty() ? 0 : mMipData[0].SizeY; }
    FORCEINLINE uint NumMipMaps() const                 { return mMipData.size(); }
    FORCEINLINE GLuint RenderResource() const           { return mTextureResource; }

    FORCEINLINE SMipData& GetMipData(uint Idx)          { return mMipData[Idx]; }

    /** @todo these functions shouldn't be handled in this class. */
    void BindToSampler(uint SamplerIndex) const;

    FORCEINLINE void SetMultisamplingEnabled(bool Enable)
    {
        mEnableMultisampling = Enable;
    }
};

#endif // CTEXTURE_H
