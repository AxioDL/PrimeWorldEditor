#ifndef CFONT_H
#define CFONT_H

#include "CResource.h"
#include "CTexture.h"
#include "TResPtr.h"
#include "Core/Resource/Model/CVertex.h"
#include "Core/OpenGL/CDynamicVertexBuffer.h"
#include "Core/OpenGL/CIndexBuffer.h"
#include <Common/BasicTypes.h>

#include <array>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#define CFONT_DEFAULT_SIZE UINT32_MAX

class CRenderer;

class CFont : public CResource
{
    DECLARE_RESOURCE_TYPE(Font)
    friend class CFontLoader;
    static std::optional<CDynamicVertexBuffer> smGlyphVertices; // This is the vertex buffer used to draw glyphs. It has two attributes - Pos and Tex0. Tex0 should be updated for each glyph.
    static CIndexBuffer smGlyphIndices; // This is the index buffer used to draw glyphs. It uses a triangle strip.
    static bool smBuffersInitialized;   // This bool indicates whether the vertex/index buffer have been initialized. Checked at the start of RenderString().

    uint32 mUnknown = 0;                // Value at offset 0x8. Not sure what this is. Including for experimentation purposes.
    uint32 mLineHeight = 0;             // Height of each line, in points
    uint32 mLineMargin = 0;             // Gap between lines, in points - this is added to the line height
    uint32 mVerticalOffset = 0;         // In points. This is used to reposition glyphs after the per-glyph vertical offset is applied
    uint32 mDefaultSize = 0;            // In points.
    TString mFontName;                  // Self-explanatory
    TResPtr<CTexture> mpFontTexture;    // The texture used by this font
    uint32 mTextureFormat = 0;          // Indicates which layers on the texture are for what - multiple glyph layers or fill/stroke

    struct SGlyph
    {
        uint16 Character;                   // The UTF-16 character that this glyph corresponds to
        std::array<CVector2f, 4> TexCoords; // The format only lists the min/max X/Y values; tracking absolute coordinates in memory is faster
        int32 LeftPadding;                  // The amount of padding applied left of this glyph, in points
        int32 RightPadding;                 // The amount of padding applied right of this glyph, in points
        uint32 Width;                       // The width of the glyph, in points
        uint32 Height;                      // The height of the glyph, in points
        uint32 PrintAdvance;                // How far the print head advances horizontally after printing this glyph, in points
        uint32 BaseOffset;                  // Vertical offset for this glyph, in points; the font-wide offset is added to this
        uint32 KerningIndex;                // Index into the kerning table of the first kerning pair for this glyph. -1 if no pairs.
        uint8 RGBAChannel;                  // Fonts can store multiple glyphs in the same space on different RGBA channels. This value corresponds to R, G, B, or A.
    };
    std::unordered_map<uint16, SGlyph> mGlyphs;

    struct SKerningPair
    {
        uint16 CharacterA;    // Left character
        uint16 CharacterB;    // Right character
        int32 Adjust;         // The horizontal offset to apply to CharacterB if this pair is encountered, in points
    };
    std::vector<SKerningPair> mKerningTable; // The kerning table should be laid out in alphabetical order for the indices to work properly


public:
    explicit CFont(CResourceEntry *pEntry = nullptr);
    ~CFont() override;
    std::unique_ptr<CDependencyTree> BuildDependencyTree() const override;
    CVector2f RenderString(const TString& rkString, CRenderer *pRenderer, float AspectRatio,
                           CVector2f Position = CVector2f(0,0),
                           CColor FillColor = CColor::White(), CColor StrokeColor = CColor::Black(),
                           uint32 FontSize = CFONT_DEFAULT_SIZE);

    // Accessors
    TString FontName() const    { return mFontName; }
    CTexture* Texture() const   { return mpFontTexture; }
private:
    static void InitBuffers();
    static void ShutdownBuffers();
};

#endif // CFONT_H
