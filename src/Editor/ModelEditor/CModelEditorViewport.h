#ifndef CMODELEDITORVIEWPORT_H
#define CMODELEDITORVIEWPORT_H

#include "Editor/CBasicViewport.h"
#include "Editor/CGridRenderable.h"
#include <Core/Scene/CModelNode.h>
#include <memory>

class CModelEditorViewport : public CBasicViewport
{
public:
    enum class EDrawMode {
        DrawMesh, DrawSphere, DrawSquare,
    };

private:
    EDrawMode mMode{EDrawMode::DrawMesh};
    CGridRenderable mGrid;
    CModelNode *mpModelNode = nullptr;
    CMaterial *mpActiveMaterial = nullptr;
    std::unique_ptr<CRenderer> mpRenderer;
    bool mGridEnabled = true;

public:
    explicit CModelEditorViewport(QWidget *pParent = nullptr);
    ~CModelEditorViewport() override;

    void SetNode(CModelNode *pNode);
    void SetActiveMaterial(CMaterial *pMat);
    void SetDrawMode(EDrawMode Mode);
    void SetClearColor(CColor Color);
    void SetGridEnabled(bool Enable);
    void Paint() override;
    void OnResize() override;
};

#endif // CMODELEDITORVIEWPORT_H
