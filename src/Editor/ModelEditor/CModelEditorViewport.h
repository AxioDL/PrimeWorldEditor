#ifndef CMODELEDITORVIEWPORT_H
#define CMODELEDITORVIEWPORT_H

#include "Editor/CBasicViewport.h"
#include "Editor/CGridRenderable.h"
#include <Core/Scene/CModelNode.h>

class CModelEditorViewport : public CBasicViewport
{
public:
    enum class EDrawMode {
        DrawMesh, DrawSphere, DrawSquare
    };

private:
    EDrawMode mMode;
    CGridRenderable mGrid;
    CModelNode *mpModelNode;
    CMaterial *mpActiveMaterial;
    CRenderer *mpRenderer;
    bool mGridEnabled;

public:
    CModelEditorViewport(QWidget *pParent = 0);
    ~CModelEditorViewport();
    void SetNode(CModelNode *pNode);
    void SetActiveMaterial(CMaterial *pMat);
    void SetDrawMode(EDrawMode Mode);
    void SetClearColor(CColor Color);
    void SetGridEnabled(bool Enable);
    void Paint();
    void OnResize();
};

#endif // CMODELEDITORVIEWPORT_H
