#include "CStaticModel.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CRenderer.h"
#include "Core/OpenGL/GLCommon.h"

CStaticModel::CStaticModel()
    : CBasicModel(nullptr)
{
}

CStaticModel::CStaticModel(CMaterial *pMat)
    : CBasicModel()
    , mpMaterial(pMat)
    , mTransparent((pMat->Options() & EMaterialOption::Transparent) != 0)
{
}

CStaticModel::~CStaticModel() = default;

void CStaticModel::AddSurface(SSurface *pSurface)
{
    mSurfaces.push_back(pSurface);
    mAABox.ExpandBounds(pSurface->AABox);

    mVertexCount += pSurface->VertexCount;
    mTriangleCount += pSurface->TriangleCount;
}

void CStaticModel::BufferGL()
{
    if (!mBuffered)
    {
        mVBO.Clear();
        mIBOs.clear();

        for (uint32 iSurf = 0; iSurf < mSurfaces.size(); iSurf++)
        {
            SSurface *pSurf = mSurfaces[iSurf];

            uint16 VBOStartOffset = (uint16) mVBO.Size();
            mVBO.Reserve((uint16) pSurf->VertexCount);

            for (uint32 iPrim = 0; iPrim < pSurf->Primitives.size(); iPrim++)
            {
                SSurface::SPrimitive *pPrim = &pSurf->Primitives[iPrim];
                CIndexBuffer *pIBO = InternalGetIBO(pPrim->Type);
                pIBO->Reserve(pPrim->Vertices.size() + 1); // Allocate enough space for this primitive, plus the restart index

                // Next step: add new vertices to the VBO and create a small index buffer for the current primitive
                std::vector<uint16> Indices(pPrim->Vertices.size());
                for (uint32 iVert = 0; iVert < pPrim->Vertices.size(); iVert++)
                    Indices[iVert] = mVBO.AddIfUnique(pPrim->Vertices[iVert], VBOStartOffset);

                // then add the indices to the IBO. We convert some primitives to strips to minimize draw calls.
                switch (pPrim->Type)
                {
                    case EPrimitiveType::Triangles:
                        pIBO->TrianglesToStrips(Indices.data(), Indices.size());
                        break;
                    case EPrimitiveType::TriangleFan:
                        pIBO->FansToStrips(Indices.data(), Indices.size());
                        break;
                    case EPrimitiveType::Quads:
                        pIBO->QuadsToStrips(Indices.data(), Indices.size());
                        break;
                    default:
                        pIBO->AddIndices(Indices.data(), Indices.size());
                        pIBO->AddIndex(0xFFFF); // primitive restart
                        break;
                }
            }

            // Make sure the number of submesh offset vectors matches the number of IBOs, then add the offsets
            while (mIBOs.size() > mSurfaceEndOffsets.size())
                mSurfaceEndOffsets.emplace_back(std::vector<uint32>(mSurfaces.size()));

            for (uint32 iIBO = 0; iIBO < mIBOs.size(); iIBO++)
                mSurfaceEndOffsets[iIBO][iSurf] = mIBOs[iIBO].GetSize();
        }

        mVBO.Buffer();

        for (uint32 iIBO = 0; iIBO < mIBOs.size(); iIBO++)
            mIBOs[iIBO].Buffer();

        mBuffered = true;
    }
}

void CStaticModel::GenerateMaterialShaders()
{
    if (mpMaterial)
        mpMaterial->GenerateShader(false);
}

void CStaticModel::ClearGLBuffer()
{
    mVBO.Clear();
    mIBOs.clear();
    mSurfaceEndOffsets.clear();
    mBuffered = false;
}

void CStaticModel::Draw(FRenderOptions Options)
{
    if (!mBuffered) BufferGL();

    mVBO.Bind();
    glLineWidth(1.f);

    auto DoDraw = [this]()
    {
        // Draw IBOs
        for (uint32 iIBO = 0; iIBO < mIBOs.size(); iIBO++)
        {
            CIndexBuffer *pIBO = &mIBOs[iIBO];
            pIBO->Bind();
            glDrawElements(pIBO->GetPrimitiveType(), pIBO->GetSize(), GL_UNSIGNED_SHORT, nullptr);
            pIBO->Unbind();
            gDrawCount++;
        }
    };

    // Bind material
    if ((Options & ERenderOption::NoMaterialSetup) == 0)
    {
        for (CMaterial* passMat = mpMaterial; passMat; passMat = passMat->GetNextDrawPass())
        {
            passMat->SetCurrent(Options);
            DoDraw();
        }
    }
    else
    {
        DoDraw();
    }

    mVBO.Unbind();
}

void CStaticModel::DrawSurface(FRenderOptions Options, uint32 Surface)
{
    if (!mBuffered) BufferGL();

    mVBO.Bind();
    glLineWidth(1.f);

    auto DoDraw = [this, Surface]()
    {
        for (uint32 iIBO = 0; iIBO < mIBOs.size(); iIBO++)
        {
            // Since there is a shared IBO for every mesh, we need two things to draw a single one: an offset and a size
            uint32 Offset = 0;
            if (Surface > 0) Offset = mSurfaceEndOffsets[iIBO][Surface - 1];
            uint32 Size = mSurfaceEndOffsets[iIBO][Surface] - Offset;

            if (!Size) continue; // The chosen submesh doesn't use this IBO

            // Now we have both, so we can draw
            mIBOs[iIBO].DrawElements(Offset, Size);
            gDrawCount++;
        }
    };

    // Bind material
    if ((Options & ERenderOption::NoMaterialSetup) == 0)
    {
        for (CMaterial* passMat = mpMaterial; passMat; passMat = passMat->GetNextDrawPass())
        {
            passMat->SetCurrent(Options);
            DoDraw();
        }
    }
    else
    {
        DoDraw();
    }

    mVBO.Unbind();
}

void CStaticModel::DrawWireframe(FRenderOptions Options, CColor WireColor /*= CColor::skWhite*/)
{
    if (!mBuffered) BufferGL();

    // Set up wireframe
    WireColor.A = 0;
    CDrawUtil::UseColorShader(WireColor);
    Options |= ERenderOption::NoMaterialSetup;
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBlendFunc(GL_ONE, GL_ZERO);

    // Draw surfaces
    for (uint32 iSurf = 0; iSurf < mSurfaces.size(); iSurf++)
        DrawSurface(Options, iSurf);

    // Cleanup
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

CMaterial* CStaticModel::GetMaterial()
{
    return mpMaterial;
}

void CStaticModel::SetMaterial(CMaterial *pMat)
{
    mpMaterial = pMat;
    mTransparent = pMat->Options().HasFlag(EMaterialOption::Transparent);
}

bool CStaticModel::IsTransparent() const
{
    return mTransparent;
}

bool CStaticModel::IsOccluder() const
{
    return mpMaterial->Options().HasFlag(EMaterialOption::Occluder);
}

CIndexBuffer* CStaticModel::InternalGetIBO(EPrimitiveType Primitive)
{
    GLenum type = GXPrimToGLPrim(Primitive);

    for (uint32 iIBO = 0; iIBO < mIBOs.size(); iIBO++)
    {
        if (mIBOs[iIBO].GetPrimitiveType() == type)
            return &mIBOs[iIBO];
    }

    mIBOs.emplace_back(CIndexBuffer(type));
    return &mIBOs.back();
}
