#include "CFont.h"
#include "Core/GameProject/CResourceStore.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CRenderer.h"

std::optional<CDynamicVertexBuffer> CFont::smGlyphVertices;
CIndexBuffer CFont::smGlyphIndices;
bool CFont::smBuffersInitialized = false;

CFont::CFont(CResourceEntry *pEntry) : CResource(pEntry)
{
}

CFont::~CFont() = default;

static constexpr float PtsToFloat(int32 Pt)
{
    // This is a bit of an arbitrary number but it works
    // 1 / (1280 / 1.333333f / 2)
    return 0.00208333f * Pt;
}

std::unique_ptr<CDependencyTree> CFont::BuildDependencyTree() const
{
    auto pOut = std::make_unique<CDependencyTree>();
    pOut->AddDependency(mpFontTexture);
    return pOut;
}

CVector2f CFont::RenderString(const TString& rkString, CRenderer* /*pRenderer*/, float /*AspectRatio*/,
                              CVector2f /*Position*/, CColor FillColor, CColor StrokeColor, uint32 FontSize)
{
    // WIP
    if (!smBuffersInitialized)
        InitBuffers();

    // Shader setup
    CShader *pTextShader = CDrawUtil::GetTextShader();
    pTextShader->SetCurrent();

    const GLuint ModelMtxLoc = pTextShader->GetUniformLocation("ModelMtx");
    const GLuint ColorLoc = pTextShader->GetUniformLocation("FontColor");
    const GLuint LayerLoc = pTextShader->GetUniformLocation("RGBALayer");
    mpFontTexture->Bind(0);
    smGlyphVertices->Bind();
    glDisable(GL_DEPTH_TEST);

    // Initialize some more stuff before we start the character loop
    CVector2f PrintHead(-1.f, 1.f);
    const CTransform4f PtScale = CTransform4f::ScaleMatrix(PtsToFloat(1));
    SGlyph *pPrevGlyph = nullptr;

    float Scale;
    if (FontSize == CFONT_DEFAULT_SIZE)
        Scale = 1.f;
    else
        Scale = static_cast<float>(FontSize) / (mDefaultSize != 0 ? mDefaultSize : 18);

    for (uint32 iChar = 0; iChar < rkString.Length(); iChar++)
    {
        // Get character, check for newline
        const char Char = rkString[iChar];

        if (Char == '\n')
        {
            pPrevGlyph = nullptr;
            PrintHead.X = -1;
            PrintHead.Y -= (PtsToFloat(mLineHeight) + PtsToFloat(mLineMargin) + PtsToFloat(mUnknown)) * Scale;
            continue;
        }

        // Get glyph
        auto iGlyph = mGlyphs.find(Char);
        if (iGlyph == mGlyphs.end())
            continue;
        SGlyph *pGlyph = &iGlyph->second;

        // Apply left padding and kerning
        PrintHead.X += PtsToFloat(pGlyph->LeftPadding) * Scale;

        if (pPrevGlyph)
        {
            if (pPrevGlyph->KerningIndex != UINT32_MAX)
            {
                for (uint32 iKern = pPrevGlyph->KerningIndex; iKern < mKerningTable.size(); iKern++)
                {
                    if (mKerningTable[iKern].CharacterA != pPrevGlyph->Character) break;
                    if (mKerningTable[iKern].CharacterB == rkString[iChar])
                    {
                        PrintHead.X += PtsToFloat(mKerningTable[iKern].Adjust) * Scale;
                        break;
                    }
                }
            }
        }

        // Add a newline if this character goes over the right edge of the screen
        if (PrintHead.X + ((PtsToFloat(pGlyph->PrintAdvance) + PtsToFloat(pGlyph->RightPadding)) * Scale) > 1)
        {
            PrintHead.X = -1;
            PrintHead.Y -= (PtsToFloat(mLineHeight) + PtsToFloat(mLineMargin) + PtsToFloat(mUnknown)) * Scale;

            if (Char == ' ') continue;
        }

        const float XTrans = PrintHead.X;
        const float YTrans = PrintHead.Y + ((PtsToFloat(pGlyph->BaseOffset * 2) - PtsToFloat(mVerticalOffset * 2)) * Scale);

        CTransform4f GlyphTransform = PtScale;
        GlyphTransform.Scale(CVector3f(static_cast<float>(pGlyph->Width) / 2, static_cast<float>(pGlyph->Height), 1.f));
        GlyphTransform.Scale(Scale);
        GlyphTransform.Translate(CVector3f(XTrans, YTrans, 0.f));

        // Get glyph layer
        uint8 GlyphLayer = pGlyph->RGBAChannel;
        if (mTextureFormat == 3)
            GlyphLayer *= 2;
        else if (mTextureFormat == 8)
            GlyphLayer = 3;

        // Load shader uniforms, buffer texture
        glUniformMatrix4fv(ModelMtxLoc, 1, GL_FALSE, (GLfloat*) &GlyphTransform);
        smGlyphVertices->BufferAttrib(EVertexAttribute::Tex0, pGlyph->TexCoords.data());

        // Draw fill
        glUniform1i(LayerLoc, GlyphLayer);
        glUniform4fv(ColorLoc, 1, &FillColor.R);
        smGlyphIndices.DrawElements();

        // Draw stroke
        if (mTextureFormat == 1 || mTextureFormat == 3 || mTextureFormat == 8)
        {
            uint8 StrokeLayer = 0;
            if (mTextureFormat == 1)
                StrokeLayer = 1;
            else if (mTextureFormat == 3)
                StrokeLayer = GlyphLayer + 1;
            else if (mTextureFormat == 8)
                StrokeLayer = GlyphLayer - 2;

            glUniform1i(LayerLoc, StrokeLayer);
            glUniform4fv(ColorLoc, 1, &StrokeColor.R);
            smGlyphIndices.DrawElements();
        }

        // Update print head
        PrintHead.X += PtsToFloat(pGlyph->PrintAdvance) * Scale;
        PrintHead.X += PtsToFloat(pGlyph->RightPadding) * Scale;
        pPrevGlyph = pGlyph;
    }

    glEnable(GL_DEPTH_TEST);
    return PrintHead;
}

void CFont::InitBuffers()
{
    smGlyphVertices.emplace();
    smGlyphVertices->SetActiveAttribs(EVertexAttribute::Position | EVertexAttribute::Tex0);
    smGlyphVertices->SetVertexCount(4);

    static constexpr std::array Vertices{
        CVector3f( 0.f,  0.f, 0.f),
        CVector3f( 2.f,  0.f, 0.f),
        CVector3f( 0.f, -2.f, 0.f),
        CVector3f( 2.f, -2.f, 0.f)
    };
    smGlyphVertices->BufferAttrib(EVertexAttribute::Position, Vertices.data());

    static constexpr std::array TexCoords{
        CVector2f(0.f, 0.f),
        CVector2f(1.f, 0.f),
        CVector2f(0.f, 1.f),
        CVector2f(1.f, 1.f)
    };
    smGlyphVertices->BufferAttrib(EVertexAttribute::Tex0, TexCoords.data());

    smGlyphIndices.Reserve(4);
    smGlyphIndices.AddIndex(0);
    smGlyphIndices.AddIndex(2);
    smGlyphIndices.AddIndex(1);
    smGlyphIndices.AddIndex(3);
    smGlyphIndices.SetPrimitiveType(GL_TRIANGLE_STRIP);

    smBuffersInitialized = true;
}

void CFont::ShutdownBuffers()
{
    if (smBuffersInitialized)
    {
        smGlyphVertices = std::nullopt;
        smBuffersInitialized = false;
    }
}
