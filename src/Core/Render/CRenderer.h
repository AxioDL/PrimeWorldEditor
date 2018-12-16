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

enum class EBloomMode
{
    NoBloom,
    Bloom,
    BloomMaps,
    FakeBloom
};

/**
 * @todo this rendering subsystem is bad and needs a rewrite
 * there's quite a lot of problems overall, but generally speaking, one of the
 * biggest problems with it is that scene nodes have too much control over how
 * they render, and the renderer doesn't have enough. for example, if a certain
 * render option is set, it should not be up to the node classes to respect that
 * option, the renderer should be able to enforce it. there's a lot of other issues
 * that make the renderer suboptimal and harder to maintain/extend than it should be.
 * this is also a more general issue but graphics stuff needs to be further abstracted
 * so that rendering code isn't directly calling OpenGL functions, ideally it should
 * just have more abstracted code that gets redirected to OpenGL at a lower level so
 * that other graphics backends could be supported in the future without needing to
 * majorly rewrite everything (but I guess that's the point we're at right now anyway).
 * I'm also pretty sure there's been no attempt made whatsoever to reduce the number of
 * shader/texture state changes needed per frame, outside batching world geometry (via
 * CStaticModel), which might be a performance drain.
 *
 * for more complaints about the rendering system implementation, see CSceneNode
 */
class CRenderer
{
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
    void AddMesh(IRenderable *pRenderable, int ComponentIndex, const CAABox& rkAABox, bool Transparent, ERenderCommand Command, EDepthGroup DepthGroup = EDepthGroup::Midground);
    void BeginFrame();
    void EndFrame();
    void ClearDepthBuffer();

    // Private
private:
    void InitFramebuffer();
};

extern uint32 gDrawCount;

#endif // RENDERMANAGER_H
