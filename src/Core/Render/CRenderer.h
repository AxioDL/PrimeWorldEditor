#ifndef CRENDERER_H
#define CRENDERER_H

#include "CCamera.h"
#include "CGraphics.h"
#include "CRenderBucket.h"
#include "EDepthGroup.h"
#include "ERenderCommand.h"
#include "FRenderOptions.h"
#include "SRenderablePtr.h"
#include "SViewInfo.h"
#include "Core/OpenGL/CFramebuffer.h"
#include "Core/Resource/CFont.h"
#include "Core/Resource/CLight.h"
#include "Core/Resource/CTexture.h"
#include "Core/Scene/CSceneNode.h"

#include <Common/CColor.h>
#include <Common/Math/CAABox.h>
#include <Common/Math/CMatrix4f.h>

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
    uint32 mContextIndex;
    bool mInitialized;
    uint32 mViewportWidth, mViewportHeight;
    uint32 mBloomWidth, mBloomHeight;
    float mBloomHScale, mBloomVScale;

    CFramebuffer mSceneFramebuffer;
    CFramebuffer mPostProcessFramebuffer;
    CFramebuffer mBloomFramebuffers[3];
    GLint mDefaultFramebuffer;

    CRenderBucket mBackgroundBucket;
    CRenderBucket mMidgroundBucket;
    CRenderBucket mForegroundBucket;
    CRenderBucket mUIBucket;

    // Static Members
    static uint32 sNumRenderers;

public:
    // Initialization
    CRenderer();
    ~CRenderer();
    void Init();

    // Accessors
    FRenderOptions RenderOptions() const;
    void ToggleBackfaceCull(bool Enable);
    void ToggleUVAnimation(bool Enable);
    void ToggleGrid(bool Enable);
    void ToggleOccluders(bool Enable);
    void ToggleAlphaDisabled(bool Enable);
    void SetBloom(EBloomMode BloomMode);
    void SetClearColor(const CColor& rkClear);
    void SetViewportSize(uint32 Width, uint32 Height);

    // Render
    void RenderBuckets(const SViewInfo& rkViewInfo);
    void RenderBloom();
    void RenderSky(CModel *pSkyboxModel, const SViewInfo& rkViewInfo);
    void AddMesh(IRenderable *pRenderable, int ComponentIndex, const CAABox& rkAABox, bool Transparent, ERenderCommand Command, EDepthGroup DepthGroup = eMidground);
    void BeginFrame();
    void EndFrame();
    void ClearDepthBuffer();

    // Private
private:
    void InitFramebuffer();
};

extern uint32 gDrawCount;

#endif // RENDERMANAGER_H
