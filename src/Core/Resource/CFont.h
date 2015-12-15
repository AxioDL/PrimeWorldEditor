#ifndef CFONT_H
#define CFONT_H

#include "CResource.h"
#include "CTexture.h"
#include "TResPtr.h"
#include "Core/Resource/Model/CVertex.h"
#include "Core/OpenGL/CDynamicVertexBuffer.h"
#include "Core/OpenGL/CIndexBuffer.h"
#include <Common/types.h>

#include <string>
#include <unordered_map>

#define CFONT_DEFAULT_SIZE -1

class CRenderer;

class CFont : public CResource
{
    DECLARE_RESOURCE_TYPE(eFont)
    friend class CFontLoader;
    static CDynamicVertexBuffer smGlyphVertices; // This is the vertex buffer used to draw glyphs. It has two attributes - Pos and Tex0. Tex0 should be updated for each glyph.
    static CIndexBuffer smGlyphIndices;          // This is the index buffer used to draw glyphs. It uses a triangle strip.
    static bool smBuffersInitialized;            // This bool indicates whether the vertex/index buffer have been initialized. Checked at the start of RenderString().

    u32 mUnknown;                    // Value at offset 0x8. Not sure what this is. Including for experimentation purposes.
    u32 mLineHeight;                 // Height of each line, in points
    u32 mLineMargin;                 // Gap between lines, in points - this is added to the line height
    u32 mVerticalOffset;             // In points. This is used to reposition glyphs after the per-glyph vertical offset is applied
    u32 mDefaultSize;                // In points.
    TString mFontName;               // Self-explanatory
    TResPtr<CTexture> mpFontTexture; // The texture used by this font
    u32 mTextureFormat;              // Indicates which layers on the texture are for what - multiple glyph layers or fill/stroke

    struct SGlyph
    {
        u16 Character;          // The UTF-16 character that this glyph corresponds to
        CVector2f TexCoords[4]; // The format only lists the min/max X/Y values; tracking absolute coordinates in memory is faster
        s32 LeftPadding;        // The amount of padding applied left of this glyph, in points
        s32 RightPadding;       // The amount of padding applied right of this glyph, in points
        u32 Width;              // The width of the glyph, in points
        u32 Height;             // The height of the glyph, in points
        u32 PrintAdvance;       // How far the print head advances horizontally after printing this glyph, in points
        u32 BaseOffset;         // Vertical offset for this glyph, in points; the font-wide offset is added to this
        u32 KerningIndex;       // Index into the kerning table of the first kerning pair for this glyph. -1 if no pairs.
        u8 RGBAChannel;         // Fonts can store multiple glyphs in the same space on different RGBA channels. This value corresponds to R, G, B, or A.
    };
    std::unordered_map<u16, SGlyph> mGlyphs;

    struct SKerningPair
    {
        u16 CharacterA;    // Left character
        u16 CharacterB;    // Right character
        s32 Adjust;        // The horizontal offset to apply to CharacterB if this pair is encountered, in points
    };
    std::vector<SKerningPair> mKerningTable; // The kerning table should be laid out in alphabetical order for the indices to work properly


public:
    CFont();
    ~CFont();
    CResource* MakeCopy(CResCache *pCopyCache);
    CVector2f RenderString(const TString& String, CRenderer *pRenderer, float AspectRatio,
                           CVector2f Position = CVector2f(0,0),
                           CColor FillColor = CColor::skWhite, CColor StrokeColor = CColor::skBlack,
                           u32 FontSize = CFONT_DEFAULT_SIZE);
private:
    void InitBuffers();
};

#endif // CFONT_H
