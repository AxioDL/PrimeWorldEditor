#ifndef CRENDERER_H
#define CRENDERER_H

#include "CCamera.h"
#include "CGraphics.h"
#include "CRenderBucket.h"
#include "FRenderOptions.h"
#include "ERenderCommand.h"
#include "SRenderablePtr.h"
#include "SViewInfo.h"
#include "Core/OpenGL/CFramebuffer.h"
#include "Core/Resource/CFont.h"
#include "Core/Resource/CLight.h"
#include "Core/Resource/CTexture.h"
#include "Core/Scene/CSceneNode.h"

#include <Common/CColor.h>
#include <Math/CAABox.h>
#include <Math/CMatrix4f.h>

#include <GL/glew.h>

class CRenderer
{
public:
    enum EBloomMode {
        eNoBloom, eBloom, eBloomMaps, eFakeBloom
    };

private:
    FRenderOptions mOptions;
    EBloomMode mBloomMode;
    bool mDrawGrid;
    CColor mClearColor;
    u32 mContextIndex;
    CRenderBucket mOpaqueBucket;
    CRenderBucket mTransparentBucket;
    bool mInitialized;
    u32 mViewportWidth, mViewportHeight;
    u32 mBloomWidth, mBloomHeight;
    float mBloomHScale, mBloomVScale;

    CFramebuffer mSceneFramebuffer;
    CFramebuffer mBloomFramebuffers[3];
    GLint mDefaultFramebuffer;

    // Static Members
    static u32 sNumRenderers;

public:
    // Initialization
    CRenderer();
    ~CRenderer();
    void Init();

    // Getters/Setters
    FRenderOptions RenderOptions() const;
    void ToggleBackfaceCull(bool b);
    void ToggleUVAnimation(bool b);
    void ToggleGrid(bool b);
    void ToggleOccluders(bool b);
    void ToggleAlphaDisabled(bool b);
    void SetBloom(EBloomMode BloomMode);
    void SetClearColor(const CColor& Clear);
    void SetViewportSize(u32 Width, u32 Height);

    // Render
    void RenderBuckets(const SViewInfo& ViewInfo);
    void RenderBloom();
    void RenderSky(CModel *pSkyboxModel, const SViewInfo& ViewInfo);
    void AddOpaqueMesh(IRenderable *pRenderable, int AssetID, CAABox& AABox, ERenderCommand Command);
    void AddTransparentMesh(IRenderable *pRenderable, int AssetID, CAABox& AABox, ERenderCommand Command);
    void BeginFrame();
    void EndFrame();
    void ClearDepthBuffer();

    // Private
private:
    void InitFramebuffer();
};

extern u32 gDrawCount;

#endif // RENDERMANAGER_H
